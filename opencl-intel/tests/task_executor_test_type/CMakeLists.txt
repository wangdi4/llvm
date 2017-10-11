# remove the restriction of warning as error for this component
if (NOT WIN32)
remove_definitions( -Werror )
endif (NOT WIN32)

link_directories( ${TBB_LIB_DIR} )

set( TARGET task_executor_test_type )

include_directories( AFTER
    ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
    ${OCL_SOURCE_DIR}/utils/task_executor)

FIND_SOURCES( )
CALCULATE_TARGET_SOURCES()

set( TARGET_LIBS cl_sys_utils
    task_executor${BUILD_PLATFORM}${OPENCL_BINARIES_POSTFIX} )

add_ocl_unittest( ${TARGET} SOURCE_FILES ${TARGET_SOURCES}
    LINK_LIBRARIES ${TARGET_LIBS} )
