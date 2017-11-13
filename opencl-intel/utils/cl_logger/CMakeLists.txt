#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME cl_logger${OPENCL_BINARIES_POSTFIX} )
include_directories( ${CL_API_HEADERS} )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

if (WIN32)
    set( LIB_TYPE STATIC  )
    set( SYSTEM_LIBS      )
else()
    # Linux
    set( LIB_TYPE SHARED  )
    set( SYSTEM_LIBS rt )
endif(WIN32)

# add a target named ${TARGET_NAME}
add_opencl_library (${TARGET_NAME} ${LIB_TYPE}
    ${TARGET_SOURCES}

    LINK_LIBS
    cl_hw_utils
    cl_sys_utils
    ${SYSTEM_LIBS}
    )

if (NOT WIN32)
    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} cl_logger_linux_exports.txt )
endif (NOT WIN32)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME} )
