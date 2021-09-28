# set that name of the main output file as a target name
set (TARGET_NAME device_commands)

include(itt)

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

# add a target named ${TARGET_NAME}
add_opencl_library (${TARGET_NAME} STATIC
    ${TARGET_SOURCES}

    INCLUDE_DIRS
    ${CL_API_HEADERS}

    INSTALL_PATH
    ${OUTPUT_ARCH_SUFF}
    )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME} )
