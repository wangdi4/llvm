use_eh(TRUE)

set(TARGET_NAME ReflectionModuleTest)

project (${TARGET_NAME})

if (NOT WIN32)
  llvm_replace_compiler_option(CMAKE_CXX_FLAGS "-Werror" "")
endif()

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${BACKEND_ROOT_DIR}/name_mangling
  ${BACKEND_ROOT_DIR}/reflection_module
  ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo/
  ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo/Driver/OpenCL
  ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo/Driver/Volcano
  ${OCL_SOURCE_DIR}/externals/gtest/googletest/include
)

add_executable(
  ${TARGET_NAME}
  TestBuiltinKeeper.cpp
)

target_link_libraries(
  ${TARGET_NAME}
  BuiltinLibInfo
  OclBackendUtils
  reflection_module
  name_mangle
  metadataapi
  gtest
  ${LLVM_MODULE_LIBS}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_TESTS_FOLDER_NAME} )

install(FILES mybi.ll DESTINATION tests/reflection_module)
install(FILES ReflectionModuleTest.pm DESTINATION tests/reflection_module)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/reflection_module)
