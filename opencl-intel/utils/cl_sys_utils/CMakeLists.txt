#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME cl_sys_utils )
 
if( DISABLE_NUMA_SUPPORT )
  add_definitions( -DDISABLE_NUMA_SUPPORT )
endif( DISABLE_NUMA_SUPPORT )

include_directories( AFTER
            ../cl_hw_utils/export
            ${CMAKE_SOURCE_DIR}/fe_compilers/clang_compiler
            )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

SET_UNICODE_OFF()

# Include the ASM sources
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (WIN32)
        set (UTILS_ASM_SOURCES kernel_execute_64.asm)
    else (WIN32)
        set (UTILS_ASM_SOURCES linux64_utils.s)
    endif (WIN32)
    CREATE_ASM_RULES( UTILS_ASM_ADD_TO_SOURCES_LIST ${UTILS_ASM_SOURCES} )
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} STATIC ${TARGET_SOURCES} ${UTILS_ASM_ADD_TO_SOURCES_LIST})

# add Version.lib in Windows for GetModuleProductVersion()
if( WIN32 )
  target_link_libraries( ${TARGET_NAME} Version.lib )
endif( WIN32 ) 

if( WIN32 OR ANDROID )
  target_link_libraries( ${TARGET_NAME} cl_hw_utils )
else( WIN32 OR ANDROID )
  target_link_libraries( ${TARGET_NAME} cl_hw_utils numa )
endif( WIN32 OR ANDROID )
 
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME} )

install( TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)

