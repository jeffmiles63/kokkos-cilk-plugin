

INSTALL (DIRECTORY
  "${CMAKE_CURRENT_LIST_DIR}/"
  DESTINATION ${KOKKOS_HEADER_DIR}
  FILES_MATCHING PATTERN "*.hpp"
)

APPEND_GLOB(KOKKOS_CORE_SRCS ${CMAKE_CURRENT_LIST_DIR}/impl/*.cpp)
APPEND_GLOB(KOKKOS_CORE_SRCS ${CMAKE_CURRENT_LIST_DIR}/CilkPlus/*.cpp)

IF (NOT KOKKOS_ENABLE_EMU)
   LIST(REMOVE_ITEM KOKKOS_CORE_SRCS ${CMAKE_CURRENT_LIST_DIR}/impl/Kokkos_EmuSpace.cpp)
ENDIF()
