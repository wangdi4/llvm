set( TARGET bi_test_type )
set( INSTALL_DIR tests/${TARGET} )

include_directories( AFTER
    ${CL_API_HEADERS}
    ../../externals/gtest/include )

add_executable( ${TARGET} bi_tests.cpp pipe_tests.cpp )
target_link_libraries( ${TARGET} googletest ${PTHREAD_LIB})
target_link_libraries( ${TARGET} OpenCL )

install( FILES
    ../../backend/libraries/ocl_builtins/pipes.h
    pipe_tests.cl
    DESTINATION ${INSTALL_DIR} )

set_target_properties( ${TARGET} PROPERTIES FOLDER "validation" )
install( TARGETS ${TARGET} DESTINATION ${INSTALL_DIR} )
