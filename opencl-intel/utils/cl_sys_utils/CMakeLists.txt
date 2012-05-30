#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME cl_sys_utils )

if (WIN32)
    # the /Ob0 flag is set instead of /Ob2 flag in order to prevent inline function  
    # expansion optimization in Visual Studio due to a bug in device fission.
    # once the bug is fixed the flag must be removed.    
    string( REPLACE /Ob2  "/Ob0" CMAKE_C_FLAGS_RELEASE   ${CMAKE_CXX_FLAGS_RELEASE} )
    string( REPLACE /Ob2  "/Ob0" CMAKE_CXX_FLAGS_RELEASE   ${CMAKE_CXX_FLAGS_RELEASE} )
endif (WIN32)

include_directories( AFTER
            ../cl_hw_utils/export
            )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

SET_UNICODE_OFF()

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} STATIC ${TARGET_SOURCES})
if (NOT WIN32)
  target_link_libraries( ${TARGET_NAME} cl_hw_utils numa )
else ()
  target_link_libraries( ${TARGET_NAME} cl_hw_utils )
endif(NOT WIN32)
 
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME} )

install( TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)

