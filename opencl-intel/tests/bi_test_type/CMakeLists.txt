set( TARGET bi_test_type )

set( TARGET_SOURCES bi_tests.cpp amx_tests.cpp atomic_add_float.cpp
                    atomic_min_max_float.cpp)
set( TARGET_LIBS test_utils intelocl${BUILD_PLATFORM} )

include_directories( ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins )

link_directories( ${TBB_LIB_DIR} )

# TODO: remove from runtime unittests, see CORC-2398
add_ocl_unittest( ${TARGET} SOURCE_FILES ${TARGET_SOURCES}
  LINK_LIBRARIES ${TARGET_LIBS} )

add_ocl_unittest_artifacts( FILES sycl_amx_builtin.spv )
