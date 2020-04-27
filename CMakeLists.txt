

KOKKOS_DEVICE_OPTION(CILKPLUS ON DEVICE "Enable CilkPlus backend")
SET(KOKKOS_CILKPLUS_NAME Experimental::CilkPlus)

##SET(Kokkos_DEFAULT_HOST_PARALLEL_EXECUTION_SPACE Experimental::CilkPlus CACHE STRING "Override default host parallel execution space")
SET(KOKKOS_CILKPLUS_FEATURE_LEVEL 10)

