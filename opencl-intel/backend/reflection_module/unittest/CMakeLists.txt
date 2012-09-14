cmake_minimum_required(VERSION 2.8)

set(TARGET_NAME ReflectionModuleTest)

project (${TARGET_NAME})

add_executable(
  ${TARGET_NAME}
  TestBuiltinKeeper.cpp
)

if (WIN32)
  add_definitions(/EHsc)
else(WIN32)
  add_definitions(-fexceptions)
endif(WIN32)

include_directories(
  ${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include
  ${CMAKE_SOURCE_DIR}/backend/name_mangling
  ../
)

target_link_libraries(
  ${TARGET_NAME}
  reflection_module
  gtest
  gtest_main
  LLVMSupport
)

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)
