add_definitions(-I${CMAKE_CURRENT_SOURCE_DIR}/../googletest/include -DGTEST_HAS_RTTI=0 -DGTEST_HAS_TR1_TUPLE=0)

add_llvm_library(UnitTestMain
  TestMain.cpp
  )