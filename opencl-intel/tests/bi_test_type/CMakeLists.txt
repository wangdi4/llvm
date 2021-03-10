set( TARGET bi_test_type )

set( TARGET_SOURCES bi_tests.cpp amx_tests.cpp )
set( TARGET_LIBS test_utils intelocl${BUILD_PLATFORM} )

link_directories( ${TBB_LIB_DIR} )

# TODO: remove from runtime unittests, see CORC-2398
add_ocl_unittest( ${TARGET} SOURCE_FILES ${TARGET_SOURCES}
  LINK_LIBRARIES ${TARGET_LIBS} )

add_ocl_unittest_artifacts( FILES sycl_amx_builtin.spv )
