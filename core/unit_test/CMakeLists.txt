
  SET(CILKPLUS_SOURCES
      ${CMAKE_CURRENT_LIST_DIR}/UnitTestMainInit.cpp
      ${CMAKE_CURRENT_LIST_DIR}/cilkplus/TestCilkPlus_Init.cpp
      ${CMAKE_CURRENT_LIST_DIR}/cilkplus/TestCilkPlus_SharedAlloc.cpp
     )

  KOKKOS_ADD_EXECUTABLE_AND_TEST(
    UnitTest_CilkPlus
    SOURCES
    ${CILKPLUS_SOURCES}
  )
