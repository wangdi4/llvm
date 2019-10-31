set( INCLUDE_HEADERS_PATH ${CCLANG_USER_INCLUDE_DIRS})

if (BUILD_X64)
set (
  SPIR_OPT
  -triple spir64-unknown-unknown -cl-kernel-arg-info -D __x86_64__
)
set ( NAME_POSTFIX _64 )
else (BUILD_X64)
set (
  SPIR_OPT
  -triple spir-unknown-unknown -cl-kernel-arg-info -D __i386__
)
set ( NAME_POSTFIX _32 )
endif(BUILD_X64)

set(TARGET_NAME backend_test_type)

# function to compile cl to bc.
function( compile INPUT_FILE OUTPUT_FILE_NAME)

  add_custom_command( TARGET ${TARGET_NAME}
    COMMAND ${CLANG} -cc1 -x cl ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
    -I ${INCLUDE_HEADERS_PATH} -emit-llvm-bc ${SPIR_OPT}
    -include opencl-c.h -O0 -disable-llvm-passes -disable-intel-proprietary-opts
    -o ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE_NAME}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
    DEPENDS ${INCLUDE_HEADERS_PATH}/opencl-c.h ${CLANG}
    COMMENT "Generate spir binary files for ${TARGET_NAME}: ${OUTPUT_FILE_NAME}"
    VERBATIM
  )

endfunction( compile )

# Set output directory of plugin targets, using `$<0:>` to avoid multi-
# configuration generators (VS, Xcode) appending a per-configuration 
# subdirectory to the specified directory.
set(BE_TESTS_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/$<0:>)

add_subdirectory(OCLSamplePlugin)

set(BACKEND_ROOT_DIR ${OCL_SOURCE_DIR}/backend)
include_directories(
    ${LLVM_INCLUDE_DIRS}
    ${OCL_SOURCE_DIR}/cl_api
    ${OCL_SOURCE_DIR}/tests/test_utils/export
    ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
    ${CL_API_HEADERS}
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend
    ${BACKEND_ROOT_DIR}/validations/Common
    ${BACKEND_ROOT_DIR}/dynamic_lib
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/plugin_manager)

set( CMAKE_CXX_FLAGS_DEBUG "-DOCL_DEV_BACKEND_PLUGINS ${CMAKE_CXX_FLAGS_DEBUG}")

add_ocl_unittest( ${TARGET_NAME}
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
    ${LLVM_MODULE_LIBS})


add_ocl_unittest_artifacts(
  FILES
    Bitcode/bitcodeNoise.bc
    Bitcode/reqd_num_sub_groups_32.bc
    Bitcode/reqd_num_sub_groups_64.bc)

# compile cl files to bc
set ( NOKERNELS_OUTPUT_NAME bitcodeNoKernels${NAME_POSTFIX}.bc )
compile( Bitcode/cltestNoKernels.cl ${NOKERNELS_OUTPUT_NAME} )
set ( WITHKERNELS_OUTPUT_NAME bitcodeWithKernels${NAME_POSTFIX}.bc )
compile( Bitcode/cltestWithKernels.cl ${WITHKERNELS_OUTPUT_NAME} )
