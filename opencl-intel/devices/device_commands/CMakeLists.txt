# set that name of the main output file as a target name
set( TARGET_NAME device_commands)
 
include( ../../cmake_utils/itt.cmake )
 
FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} STATIC ${TARGET_SOURCES})

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME} )

install( TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)
