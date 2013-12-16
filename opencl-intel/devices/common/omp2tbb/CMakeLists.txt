#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME omp2tbb )

include( ${OCL_SOURCE_DIR}/cmake_utils/itt.cmake )

include_directories( BEFORE
                        ${HW_UTILS_HEADERS} 
                    )

if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions( -DRESOURCE_X64 )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8) 
    set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${IMPLIB_SUBDIR}/VS2010/${OCL_SYS_DEPENDENT_SUBDIR} )
else (WIN32)
    set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )
endif (WIN32)

link_directories( ${TBB_LIB_DIR} )

add_definitions( -D_USRDLL -DTBB_PREVIEW_LOCAL_OBSERVER=1 -DTBB_PREVIEW_TASK_ARENA=1 -D__TBB_NO_IMPLICIT_LINKAGE=1)

SET_UNICODE_OFF()

#Ignore assembly files
set( IGNORE_FILES z_ )

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

if (WIN32)
    set (ASM_SRC_FILE z_Windows_NT-586_asm.asm)
    set (OBJ_FILE_NATIVE z_Windows_NT-586_asm.obj)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set (DEVICE_ASSEMBLER "ml64.exe")
        set (ASM_ARCH "_M_AMD64")
    else(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set (DEVICE_ASSEMBLER "ml.exe")
        set (ASM_ARCH "_M_IA32")
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8) 
    set (ASM_COMMAND_LINE ${DEVICE_ASSEMBLER} /nologo /D${ASM_ARCH}=1 /Fo ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}/${OBJ_FILE_NATIVE} /c ${CMAKE_CURRENT_SOURCE_DIR}/${ASM_SRC_FILE} )
else (WIN32)
    set (ASM_SRC_FILE z_Linux_asm.s)
    set (OBJ_FILE_NATIVE z_Linux_asm.o)
    set (ASM_ARCH "KMP_ARCH_X86_64")
    # Before compilation need to run though C pre-processor since GAS doesn't have preprocessor
    set (DEVICE_ASSEMBLER "gcc -E -D${ASM_ARCH}=1 ${CMAKE_CURRENT_SOURCE_DIR}/${ASM_SRC_FILE} | as")
    set (ASM_COMMAND_LINE ${DEVICE_ASSEMBLER} -o ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}/${OBJ_FILE_NATIVE} )
endif(WIN32)

add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}/${OBJ_FILE_NATIVE}
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}
                    COMMAND ${ASM_COMMAND_LINE}
                    MAIN_DEPENDENCY ${ASM_SRC_FILE}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    COMMENT "Compiling assembly ${ASM_SRC_FILE}"
                    VERBATIM
                    )
list (APPEND TARGET_SOURCES ${ASM_SRC_FILE})

#add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

install( TARGETS ${TARGET_NAME}     
            RUNTIME DESTINATION bin
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION bin)

if (WIN32)
    list(APPEND LINK_LIBS delayimp.lib )
else (WIN32)
    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} omp2lib_linux_exports.txt )
endif(WIN32)

if (WIN32)
    get_target_property(RT_OUTPUT_DIRECTORY ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
    file(TO_NATIVE_PATH ${RT_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${TARGET_NAME}_stripped.pdb PDB_NAME)
    # Visual Studio 2010 (assumed if not Visual Studio 2008)
    # This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the linker flags in Release mode.
    # The /DEBUG flag is required in order to create stripped pdbs.    
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG    "${LINK_FLAGS_DEBUG} tbb_debug.lib /DELAYLOAD:tbb_debug.dll /PDBSTRIPPED:${PDB_NAME}")
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE    "${LINK_FLAGS_RELEASE} tbb.lib /DELAYLOAD:tbb.dll /DEBUG /PDBSTRIPPED:${PDB_NAME}")
    install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}_stripped.pdb DESTINATION bin)
    if (INSTALL_PDBS)
        install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}.pdb DESTINATION bin)
    endif(INSTALL_PDBS)
else (WIN32)
    if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
        list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb_debug${IMPLIB_SUFFIX} )
        if( NOT ANDROID )
          list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbbmalloc_debug${IMPLIB_SUFFIX} )
        endif( NOT ANDROID )
    else ()
        list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb${IMPLIB_SUFFIX} )
        if( NOT ANDROID )
          list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbbmalloc${IMPLIB_SUFFIX} )
        endif( NOT ANDROID )
    endif ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
endif (WIN32)
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME} )
