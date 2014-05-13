#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME isp_device${BUILD_PLATFORM} )

include_directories( BEFORE
    ${OCL_BINARY_DIR}
    css
    css/hrt )

if (WIN32)
    set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${IMPLIB_SUBDIR}/VS2010/${OCL_SYS_DEPENDENT_SUBDIR} )
else (WIN32)
    set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )
endif (WIN32)

link_directories( ${TBB_LIB_DIR}
                  ${CMAKE_CURRENT_SOURCE_DIR}/res )


# TODO: remove this
if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions( -DRESOURCE_X64 )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
endif (WIN32)

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

FILE(GLOB CSS_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} css/*.h)
FILE(GLOB CAMERASHIM_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} res/*.h)

if (WIN32)
    source_group( "CSS Headers" FILES ${CSS_HEADERS} )
    list( APPEND TARGET_SOURCES ${CSS_HEADERS} )
    
    source_group( "CameraShim" FILES ${CAMERASHIM_HEADERS} )
    list( APPEND TARGET_SOURCES ${CAMERASHIM_HEADERS} )
endif (WIN32)

#add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

if (NOT WIN32)
    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} isp_device_linux_exports.txt )
endif (NOT WIN32)

target_link_libraries( ${TARGET_NAME}
    cl_sys_utils
    cl_logger
    task_executor${BUILD_PLATFORM}
    shimOCL )

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME} )

if (WIN32)
    get_target_property(RT_OUTPUT_DIRECTORY ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
    file(TO_NATIVE_PATH ${RT_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${TARGET_NAME}_stripped.pdb PDB_NAME)
    # Visual Studio 2010
    # This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the linker flags in Release mode.
    # The /DEBUG flag is required in order to create stripped pdbs.
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG    "${LINK_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:MSVCPRTD /NODEFAULTLIB:MSVCRTD /PDBSTRIPPED:${PDB_NAME}")
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE  "${LINK_FLAGS_RELEASE} /DEBUG /PDBSTRIPPED:${PDB_NAME}")

    install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}_stripped.pdb DESTINATION bin)

    if (INSTALL_PDBS)    
        install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}.pdb DESTINATION bin)
    endif(INSTALL_PDBS)
endif(WIN32)
