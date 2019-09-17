# remove the restriction of warning as error for this component
if (NOT WIN32)
remove_definitions( -Werror )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error")
endif (NOT WIN32)

link_directories( ${TBB_LIB_DIR} )

set( TARGET task_executor_test_type )

include_directories( AFTER
    ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
    ${OCL_SOURCE_DIR}/utils/task_executor)

FIND_SOURCES( )
CALCULATE_TARGET_SOURCES()

set( TARGET_LIBS cl_sys_utils task_executor${BUILD_PLATFORM} )

add_ocl_unittest( ${TARGET} SOURCE_FILES ${TARGET_SOURCES}
    LINK_LIBRARIES ${TARGET_LIBS} )
