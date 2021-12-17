set( TARGET bi_test_type )

set(IGNORE_FILES pipe_tests.cpp)

FIND_SOURCES(${IGNORE_FILES})
CALCULATE_TARGET_SOURCES()

set( TARGET_LIBS test_utils intelocl${BUILD_PLATFORM} )

include_directories(${OCL_SOURCE_DIR}/tests/framework_test_type
                    ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins )

link_directories( ${TBB_LIB_DIR} )

# TODO: remove from runtime unittests, see CORC-2398
add_ocl_unittest(${TARGET}
  SOURCE_FILES
    ${TARGET_SOURCES}
    ${OCL_SOURCE_DIR}/tests/framework_test_type/TestsHelpClasses.cpp
  LINK_LIBRARIES
    ${TARGET_LIBS}
  )

add_ocl_unittest_artifacts( FILES sycl_amx_builtin.spv )
