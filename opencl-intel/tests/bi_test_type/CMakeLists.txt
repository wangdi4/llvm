set( TARGET bi_test_type )

set( TARGET_SOURCES bi_tests.cpp pipe_tests.cpp )
set( TARGET_LIBS test_utils OpenCL )

# TODO: remove from runtime unittests, see CORC-2398
add_ocl_unittest( ${TARGET} SOURCE_FILES ${TARGET_SOURCES}
  LINK_LIBRARIES ${TARGET_LIBS} )

add_ocl_unittest_artifacts( FILES pipe_tests.cl
    ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/pipes-internal.h)
