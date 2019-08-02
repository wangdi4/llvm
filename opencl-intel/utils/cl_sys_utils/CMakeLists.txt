#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME cl_sys_utils )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

SET_UNICODE_OFF()

set_opencl_version()

# Include the ASM sources
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (WIN32)
        CREATE_ASM_RULES( UTILS_ASM_ADD_TO_SOURCES_LIST kernel_execute_64.asm )
    else (WIN32)
        set (UTILS_ASM_ADD_TO_SOURCES_LIST linux64_utils.s)
    endif (WIN32)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

# add a target named ${TARGET_NAME}
add_opencl_library (${TARGET_NAME} STATIC
    ${TARGET_SOURCES}
    ${UTILS_ASM_ADD_TO_SOURCES_LIST}

    INCLUDE_DIRS
    ${CCLANG_DEV_INCLUDE_DIRS}
    ${CL_API_HEADERS}
    ${OCL_BINARY_DIR}
    ${OCL_SOURCE_DIR}/utils/cl_hw_utils/export
    ${OCL_SOURCE_DIR}/FrontendDriver
    ${OCL_SOURCE_DIR}/tools/OCLProfiler/Inc
    )

if ( USE_NUMA AND NOT WIN32 )
    add_definitions( -DUSE_NUMA=1 )
    target_link_libraries( ${TARGET_NAME} numa )
endif( USE_NUMA AND NOT WIN32 )

# add Version.lib in Windows for GetModuleProductVersion()
if( WIN32 )
    target_link_libraries( ${TARGET_NAME} Version.lib cl_hw_utils )
else( WIN32 )
    target_link_libraries( ${TARGET_NAME} cl_hw_utils rt )
endif( WIN32 )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME} )
