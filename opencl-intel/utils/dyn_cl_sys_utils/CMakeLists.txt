if(WIN32)
    ocl_replace_compiler_option_to_dynamic()
endif(WIN32)
#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME dyn_cl_sys_utils )
 
FIND_SOURCES_IN_FOLDER(../cl_sys_utils)
CALCULATE_TARGET_SOURCES()

SET_UNICODE_OFF()

# Include the ASM sources
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (WIN32)
        CREATE_ASM_RULES( UTILS_ASM_ADD_TO_SOURCES_LIST ../cl_sys_utils/kernel_execute_64.asm )
    else (WIN32)
        set (UTILS_ASM_ADD_TO_SOURCES_LIST ../cl_sys_utils/linux64_utils.s)
    endif (WIN32)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

# add a target named ${TARGET_NAME}
add_opencl_library (${TARGET_NAME} STATIC
    ${TARGET_SOURCES}
    ${UTILS_ASM_ADD_TO_SOURCES_LIST}

    INCLUDE_DIRS
    ../cl_hw_utils/export
    ${CCLANG_USER_INCLUDE_DIRS}
    ${CL_API_HEADERS}
    ${OCL_BINARY_DIR}
    ${OCL_SOURCE_DIR}/FrontendDriver
    ${OCL_SOURCE_DIR}/tools/OCLProfiler/Inc
    )

if ( USE_NUMA AND NOT WIN32 )
    add_definitions( -DUSE_NUMA=1 )
    target_link_libraries( ${TARGET_NAME} numa )
endif( USE_NUMA AND NOT WIN32 )

# add Version.lib in Windows for GetModuleProductVersion()
if( WIN32 )
    target_link_libraries( ${TARGET_NAME} Version.lib dyn_cl_hw_utils )
else( WIN32 )
    target_link_libraries( ${TARGET_NAME} dyn_cl_hw_utils )
endif( WIN32 )
 
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME} )
