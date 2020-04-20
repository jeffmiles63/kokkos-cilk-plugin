
CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/KokkosCore_config_cilkplus.h.in KokkosCore_config_cilkplus.h @ONLY)

LIST(APPEND KOKKOS_INSTALL_PLUGIN_LIST KokkosCore_config_cilkplus.h)


