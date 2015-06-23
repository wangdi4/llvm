# set that name of the main output file as a target name
set( TARGET_NAME device_commands)
 
include( ../../cmake_utils/itt.cmake )
 
FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} STATIC ${TARGET_SOURCES})

# [LLVM 3.6 UPGRADE] Disable Linux warning about unused function
# this warning occur when processing externals/itt/include/ittnotify.h
# since it contain __itt_id __itt_id_make(void*, long long unsigned int)
# function that not use in every build target.
if (NOT WIN32)
      # disable warnings inherited from external componennts
      add_definitions( "-Wno-unused-function" )
      add_definitions( "-Wno-attributes" )
endif(NOT Win32)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME} )

install( TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)
