cmake_minimum_required(VERSION 2.8.4)

set(TARGET_NAME TestTypeConversion)

include_directories(
  ${CMAKE_SOURCE_DIR}/externals/gtest/include
  ${CMAKE_SOURCE_DIR}/backend/utils
  ${CMAKE_SOURCE_DIR}/backend/llvm/include
  ${CMAKE_SOURCE_DIR}/backend
)

add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS)

add_llvm_executable(${TARGET_NAME}
  TestTypeConversion.cpp
)

target_link_libraries(
  ${TARGET_NAME}
  antlr
  name_mangle
  LLVMSupport
  LLVMCore
  OclUtils
  googletest
)

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)
