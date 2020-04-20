#include <KokkosCore_config_cilkplus.h>

namespace Experimental {

#if defined(KOKKOS_ENABLE_EMU)
   class EmuLocalSpace;       /// Memory space for emu local (scratch)
   class EmuReplicatedSpace;  /// Memory space for emu replicated arch (const)
   class EmuStridedSpace;     /// Memory space for emu strided arch
#endif

class CilkPlus;

}
