if(WIN32)
    ocl_replace_compiler_option_to_dynamic()
endif(WIN32)
#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME dyn_cl_hw_utils )

if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        file(GLOB ASM_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ../cl_hw_utils/win32Imp/bin32/*.asm )
    else ()
        file(GLOB ASM_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ../cl_hw_utils/win32Imp/bin64/*.asm )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 4)

    CREATE_ASM_RULES( ASM_ADD_TO_SOURCES_LIST ${ASM_SOURCES} )

else ()
    # Linux
    file(GLOB ASM_ADD_TO_SOURCES_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ../cl_hw_utils/linuxImp/*.cpp )
endif (WIN32)

FIND_SOURCES_IN_FOLDER(../cl_hw_utils)
CALCULATE_TARGET_SOURCES()

# add a target named ${TARGET_NAME}
add_opencl_library (${TARGET_NAME} STATIC ${TARGET_SOURCES} ${ASM_ADD_TO_SOURCES_LIST})

set_target_properties( ${TARGET_NAME} PROPERTIES LINKER_LANGUAGE C )
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME} )

