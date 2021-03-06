/*
//@HEADER
// ************************************************************************
//
//                        Kokkos v. 2.0
//              Copyright (2014) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact  H. Carter Edwards (hcedwar@sandia.gov)
//
// ************************************************************************
//@HEADER
*/

#include <Kokkos_Core.hpp>
#if defined( KOKKOS_ENABLE_CILKPLUS )

#include <cstdlib>
#include <sstream>

/*--------------------------------------------------------------------------*/

namespace Kokkos {
namespace Impl {

#ifdef DEBUG_TASK_COUNT	 
static int next_node_id = 0;
static int current_node_cnt = 0;
#endif
	
extern int get_next_node_id() {
#ifdef DEBUG_TASK_COUNT	  	
	return next_node_id++;
#else
    return 0;
#endif
}

extern void update_current_task_count( int val ) {
#ifdef DEBUG_TASK_COUNT	  	
	current_node_cnt += val;
#endif
}

extern void set_current_task_count( int val ) {
#ifdef DEBUG_TASK_COUNT	  		
	current_node_cnt = val;
#endif
}

extern int get_current_node_count() {
#ifdef DEBUG_TASK_COUNT	  	
	return current_node_cnt;
#else
    return 0;
#endif
}

namespace {

HostThreadTeamData g_cilkplus_thread_team_data ;

bool g_cilkplus_is_initialized = false;

}

// Resize thread team data scratch memory
void cilkplus_resize_thread_team_data( size_t pool_reduce_bytes
                                   , size_t team_reduce_bytes
                                   , size_t team_shared_bytes
                                   , size_t thread_local_bytes )
{
  if ( pool_reduce_bytes < 512 ) pool_reduce_bytes = 512 ;
  if ( team_reduce_bytes < 512 ) team_reduce_bytes = 512 ;

  const size_t old_pool_reduce  = g_cilkplus_thread_team_data.pool_reduce_bytes();
  const size_t old_team_reduce  = g_cilkplus_thread_team_data.team_reduce_bytes();
  const size_t old_team_shared  = g_cilkplus_thread_team_data.team_shared_bytes();
  const size_t old_thread_local = g_cilkplus_thread_team_data.thread_local_bytes();
  const size_t old_alloc_bytes  = g_cilkplus_thread_team_data.scratch_bytes();

  // Allocate if any of the old allocation is tool small:

  const bool allocate = ( old_pool_reduce  < pool_reduce_bytes ) ||
                        ( old_team_reduce  < team_reduce_bytes ) ||
                        ( old_team_shared  < team_shared_bytes ) ||
                        ( old_thread_local < thread_local_bytes );

  if ( allocate ) {

    Kokkos::HostSpace space ;

    if ( old_alloc_bytes ) {
      g_cilkplus_thread_team_data.disband_team();
      g_cilkplus_thread_team_data.disband_pool();

      space.deallocate( g_cilkplus_thread_team_data.scratch_buffer()
                      , g_cilkplus_thread_team_data.scratch_bytes() );
    }

    if ( pool_reduce_bytes < old_pool_reduce ) { pool_reduce_bytes = old_pool_reduce ; }
    if ( team_reduce_bytes < old_team_reduce ) { team_reduce_bytes = old_team_reduce ; }
    if ( team_shared_bytes < old_team_shared ) { team_shared_bytes = old_team_shared ; }
    if ( thread_local_bytes < old_thread_local ) { thread_local_bytes = old_thread_local ; }

    const size_t alloc_bytes =
      HostThreadTeamData::scratch_size( pool_reduce_bytes
                                      , team_reduce_bytes
                                      , team_shared_bytes
                                      , thread_local_bytes );

    void * const ptr = space.allocate( alloc_bytes );

    g_cilkplus_thread_team_data.
      scratch_assign( ((char *)ptr)
                    , alloc_bytes
                    , pool_reduce_bytes
                    , team_reduce_bytes
                    , team_shared_bytes
                    , thread_local_bytes );

    HostThreadTeamData * pool[1] = { & g_cilkplus_thread_team_data };

    g_cilkplus_thread_team_data.organize_pool( pool , 1 );
    g_cilkplus_thread_team_data.organize_team(1);
  }
}

HostThreadTeamData * cilkplus_get_thread_team_data()
{
  return & g_cilkplus_thread_team_data ;
}

} // namespace Impl
} // namespace Kokkos

/*--------------------------------------------------------------------------*/

namespace Kokkos {
namespace Experimental {

bool CilkPlus::impl_is_initialized()
{
  return Kokkos::Impl::g_cilkplus_is_initialized ;
}

void CilkPlus::impl_initialize( unsigned threads_count
                       , unsigned use_numa_count
                       , unsigned use_cores_per_numa
                       , bool allow_asynchronous_threadpool )
{
  (void) threads_count;
  (void) use_numa_count;
  (void) use_cores_per_numa;
  (void) allow_asynchronous_threadpool;

  Kokkos::Impl::SharedAllocationRecord< void, void >::tracking_enable();

  // Init the array of locks used for arbitrarily sized atomics
  Kokkos::Impl::init_lock_array_host_space();
  #if defined(KOKKOS_ENABLE_PROFILING)
    Kokkos::Profiling::initialize();
  #endif

  Kokkos::Impl::g_cilkplus_is_initialized = true;
}

void CilkPlus::impl_finalize()
{
  if ( Kokkos::Impl::g_cilkplus_thread_team_data.scratch_buffer() ) {
    Kokkos::Impl::g_cilkplus_thread_team_data.disband_team();
    Kokkos::Impl::g_cilkplus_thread_team_data.disband_pool();

    Kokkos::HostSpace space ;

    space.deallocate( Kokkos::Impl::g_cilkplus_thread_team_data.scratch_buffer()
                    , Kokkos::Impl::g_cilkplus_thread_team_data.scratch_bytes() );

    Kokkos::Impl::g_cilkplus_thread_team_data.scratch_assign( (void*) 0, 0, 0, 0, 0, 0 );
  }

  #if defined(KOKKOS_ENABLE_PROFILING)
    Kokkos::Profiling::finalize();
  #endif

  Kokkos::Impl::g_cilkplus_is_initialized = false;
}

const char* CilkPlus::name() { return "CilkPlus"; }

} // namespace Experimental
	
namespace Impl {

CilkPlusBackendSpaceFactory g_cilkplus_backend_initializer;

CilkPlusBackendSpaceFactory::CilkPlusBackendSpaceFactory() {
   BackendInitializer::get_instance().register_backend("200_CilkPlus", this);
}

void CilkPlusBackendSpaceFactory::initialize(const InitArguments& args) {
  // Prevent "unused variable" warning for 'args' input struct.  If
  // Serial::initialize() ever needs to take arguments from the input
  // struct, you may remove this line of code.
  (void)args;

  // Always initialize Serial if it is configure time enabled
  Kokkos::Experimental::CilkPlus::impl_initialize();
}

}  // namespace Impl
} // namespace Kokkos

#else
void KOKKOS_CORE_SRC_IMPL_CILKPLUS_PREVENT_LINK_ERROR() {}
#endif // defined( KOKKOS_ENABLE_CILKPLUS )

