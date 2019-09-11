# The conformance tests must be compiled with the headers located in
# ${OCL_CONFORMANCE_INCLUDE} and not the ones in ${OCL_GLOBAL_INCLUDE}
# Therefore, it must be added to the include directories only after
# the conformance subdirectory has been added.
include_directories( BEFORE ${OCL_GLOBAL_INCLUDE} )

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

set(CLANG "${LLVM_BINARY_DIR}/clang" )
set(SPIRV_CONVERTER  "${LLVM_PATH_BE}/bin/llvm-spirv")

include(itt)

# LIT infrastructure configuration
if (NOT OPENCL_INTREE_BUILD)
  include(${LLVM_PATH}/lib/cmake/llvm/AddLLVM.cmake)
endif()

# It is used by lit.cfg to find lit.site.cfg when user launches LIT tests from a
# certain sub-directory instead of launching 'check-ocl-runtime' target
set(RT_TEST_PARAMS
  rt_lit_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
)

# -s: Show less output, for example, don't show information on tests that pass.
# -v: Show more information on tests failures, for example the entrie test
# output instead of just the test result.
set(RT_TEST_ARGS ${RT_TEST_ARGS} "-s" "-v")

set(RT_TESTS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# We need to pass some cmake variables into lit .cfg scripts
configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
)

# Unittests
add_subdirectory( test_utils )
include_directories( test_utils/export )

if (NOT ${ADDR} STREQUAL "32")
  add_subdirectory( fpga_test_type )
endif (NOT ${ADDR} STREQUAL "32")
add_subdirectory( tbb_native_test_type )
add_subdirectory( cpu_device_test_type )
add_subdirectory( framework_test_type )
add_subdirectory( FrontendDriver_test_type )
add_subdirectory( task_executor_test_type )
add_subdirectory( bi_test_type )
add_subdirectory( backend_test_type )
add_subdirectory( name_mangling_test_type )
add_subdirectory( metadata_api_test_type )

if( USE_VALGRIND )
  add_subdirectory( valgrind )
endif( USE_VALGRIND )

if (INCLUDE_CMRT)
  message (STATUS "Adding Common runtime tests ...")
  if (NOT BUILD_X64)
    add_subdirectory ( common_runtime_test_type ) # until compilation errors are resolved
  endif (NOT BUILD_X64)
endif(INCLUDE_CMRT)

# Main unittests target

list(APPEND RUNTIME_UNITTESTS
  framework_test_type
  task_executor_test_type
  FrontendDriver_test_type
  cpu_device_test_type
  backend_test_type
  name_mangling_test_type
  metadata_api_test_type
)
if (NOT ${ADDR} STREQUAL "32")
  list(APPEND RUNTIME_UNITTESTS fpga_test_type )
endif (NOT ${ADDR} STREQUAL "32")

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
