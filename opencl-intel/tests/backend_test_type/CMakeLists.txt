set(TARGET_NAME backend_test_type)

# Set output directory of plugin targets, using `$<0:>` to avoid multi-
# configuration generators (VS, Xcode) appending a per-configuration
# subdirectory to the specified directory.
set(BE_TESTS_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/$<0:>)

add_subdirectory(OCLSamplePlugin)

set(BACKEND_ROOT_DIR ${OCL_SOURCE_DIR}/backend)
include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${OCL_SOURCE_DIR}/cl_api
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
  ${CL_API_HEADERS}
  ${BACKEND_ROOT_DIR}/utils
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend
  ${BACKEND_ROOT_DIR}/validations/Common
  ${BACKEND_ROOT_DIR}/dynamic_lib
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
  ${BACKEND_ROOT_DIR}/arch_headers
  ${BACKEND_ROOT_DIR}/plugin_manager)

set(CMAKE_CXX_FLAGS_DEBUG "-DOCL_DEV_BACKEND_PLUGINS ${CMAKE_CXX_FLAGS_DEBUG}")

# compile cl files to bc
if(BUILD_X64)
  set(NAME_POSTFIX _64)
else(BUILD_X64)
  set(NAME_POSTFIX _32)
endif(BUILD_X64)
set(NOKERNELS_OUTPUT_NAME bitcodeNoKernels${NAME_POSTFIX}.bc)
set(WITHKERNELS_OUTPUT_NAME bitcodeWithKernels${NAME_POSTFIX}.bc)
compile_bitcode(Bitcode/cltestNoKernels.cl ${NOKERNELS_OUTPUT_NAME}
                EXTRA_CLANG_FLAGS -cl-kernel-arg-info)
compile_bitcode(Bitcode/cltestWithKernels.cl ${WITHKERNELS_OUTPUT_NAME}
                EXTRA_CLANG_FLAGS -cl-kernel-arg-info)

set(TEST_BC_FILES ${TARGET_NAME}_test_files)
add_custom_target(${TEST_BC_FILES} DEPENDS ${NOKERNELS_OUTPUT_NAME}
                                           ${WITHKERNELS_OUTPUT_NAME})

add_ocl_unittest(
  ${TARGET_NAME}
  SOURCE_FILES
  BackendWrapper.cpp
  BWOptions.cpp
  FactoryTest.cpp
  CompilationServiceTest.cpp
  PluginsTest.cpp
  KernelSubGroupInfoTest.cpp
  CompilationUtilsTest.cpp
  LINK_LIBRARIES
  OclBackendUtils
  OclPluginManager
  dynamic_load
  cl_sys_utils
  test_utils
  LLVMCore)

add_ocl_unittest_artifacts(
  FILES Bitcode/bitcodeNoise.bc Bitcode/reqd_num_sub_groups_32.bc
  Bitcode/reqd_num_sub_groups_64.bc)

add_dependencies(${TARGET_NAME} OclCpuBackEnd${BUILD_PLATFORM} OCLSamplePlugin
                 ${OCL_RT_TEST_DEPS} ${TEST_BC_FILES})
