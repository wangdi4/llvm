#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME cl_hw_utils )

if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        file(GLOB ASM_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} win32Imp/bin32/*.asm )
    else ()
	#	message(FATAL_ERROR "Windows 64 bit support not ready")	
        file(GLOB ASM_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} win32Imp/bin64/*.asm )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 4)
else ()
    # Linux
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        file(GLOB ASM_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} linuxImp/bin32/*.S )
    else ()
        file(GLOB ASM_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} linuxImp/bin64/*.S )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 4)
endif (WIN32)

CREATE_ASM_RULES( ASM_ADD_TO_SOURCES_LIST ${ASM_SOURCES} )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} STATIC ${TARGET_SOURCES} ${ASM_ADD_TO_SOURCES_LIST})

set_target_properties( ${TARGET_NAME} PROPERTIES LINKER_LANGUAGE C )
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME} )

