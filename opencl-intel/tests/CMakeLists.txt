# The conformance tests must be compiled with the headers located in
# ${OCL_CONFORMANCE_INCLUDE} and not the ones in ${OCL_GLOBAL_INCLUDE}
# Therefore, it must be added to the include directories only after
# the conformance subdirectory has been added.
include_directories( BEFORE ${OCL_GLOBAL_INCLUDE} )

# define INTEL_CUSTOMIZATION xmain macro
add_definitions( -DINTEL_CUSTOMIZATION )

# some tests have to use deprecated OpenCL API functions.
if (WIN32)
    add_definitions(
        -wd4996 # Suppress 'function': was declared deprecated'
    )
endif (WIN32)

if( NOT DEFINED LLVM_PATH_FE )
  message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for frontend using LLVM_PATH_FE parameter to CMAKE" )
endif()

set(LLVM_PATH ${LLVM_PATH_FE})
find_package(LLVM REQUIRED)

if( CMAKE_CROSSCOMPILING OR LLVM_USE_NATIVE OR ANDROID )
  set(TOOLS_BINARY_DIR "${LLVM_BINARY_DIR}/../native/bin")
  set(SPIRV_CONVERTER  "${LLVM_PATH_BE}/native/bin/llvm-spirv")
else()
  set(TOOLS_BINARY_DIR "${LLVM_BINARY_DIR}" )
  set(SPIRV_CONVERTER  "${LLVM_PATH_BE}/bin/llvm-spirv")
endif()

set( CLANG "${TOOLS_BINARY_DIR}/clang" )

include(${OCL_SOURCE_DIR}/cmake_utils/CMakeFuncs.txt)
include(${OCL_SOURCE_DIR}/cmake_utils/itt.cmake)

# LIT infrastructure configuration
include(${LLVM_PATH}/lib/cmake/llvm/AddLLVM.cmake)

# It is used by lit.cfg to find lit.site.cfg when user launches LIT tests from a
# certain sub-directory instead of launching 'check-ocl-runtime' target
set(RT_TEST_PARAMS
  rt_lit_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
)

# -s: Show less output, for example, don't show information on tests that pass.
# -v: Show more information on tests failures, for example the entrie test
# output instead of just the test result.
set(RT_TEST_ARGS ${RT_TEST_ARGS} "-s" "-v")

set(LLVM_BINARY_DIR ${LLVM_PATH}/bin)

# Let's help CMake to find llvm/utils/lit/lit.py
set(LLVM_MAIN_SRC_DIR "${OCL_SOURCE_DIR}/../../")

set(RT_TESTS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# We need to pass some cmake variables into lit .cfg scripts
configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
)

# Unittests
add_subdirectory( test_utils )
include_directories( test_utils/export )

add_subdirectory( tbb_native_test_type )
add_subdirectory( cpu_device_test_type )
add_subdirectory( framework_test_type )
add_subdirectory( clang_compiler_test_type )

add_subdirectory( task_executor_test_type )
add_subdirectory( bi_test_type )

if ( INCLUDE_MIC_DEVICE )
  add_subdirectory( mic_device_test_type )
endif( INCLUDE_MIC_DEVICE )

if( USE_VALGRIND )
  add_subdirectory( valgrind )
endif( USE_VALGRIND )

if (INCLUDE_CMRT)
  message (STATUS "Adding Common runtime tests ...")
  if (NOT ANDROID OR NOT BUILD_X64)
    add_subdirectory ( common_runtime_test_type ) # until compilation errors are resolved
  endif (NOT ANDROID OR NOT BUILD_X64)
endif(INCLUDE_CMRT)

#Samples
option(INCLUDE_SMPLS "Include Samples into Build"  OFF)
if (INCLUDE_SMPLS)
  add_subdirectory( samples )
endif(INCLUDE_SMPLS)

# Main unittests target

list(APPEND RUNTIME_UNITTESTS
  framework_test_type
  task_executor_test_type
  clang_compiler_test_type
  cpu_device_test_type
)

list(APPEND RT_TEST_DEPS
  ${RUNTIME_UNITTESTS}
)

add_lit_testsuite(check-ocl-runtime "Running the OpenCL Runtime unittests"
  ${CMAKE_CURRENT_BINARY_DIR}
  PARAMS ${RT_TEST_PARAMS}
  DEPENDS ${RT_TEST_DEPS}
  ARGS ${RT_TEST_ARGS}
)

set_target_properties(check-ocl-runtime PROPERTIES FOLDER "Tests")
