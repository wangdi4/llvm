
# set that name of the main output file as a target name
set( TARGET_NAME isp_device${BUILD_PLATFORM} )

include_directories( BEFORE ${OCL_BINARY_DIR})
set( FILES_TO_COPY ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX} )

if (WIN32)
    add_definitions( -D_WINDOWS )
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions( -DRESOURCE_X64 )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
else ()
    add_definitions( -D_LINUX )
endif (WIN32)

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

#add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

if (NOT WIN32)
    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} isp_device_linux_exports.txt )
endif (NOT WIN32)

install( TARGETS ${TARGET_NAME}
            RUNTIME DESTINATION bin
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION bin)

target_link_libraries( ${TARGET_NAME} cl_sys_utils )
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
