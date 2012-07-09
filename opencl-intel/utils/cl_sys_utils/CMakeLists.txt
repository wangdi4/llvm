#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME cl_sys_utils )

include_directories( AFTER
            ../cl_hw_utils/export
			${CMAKE_CURRENT_SOURCE_DIR}/../../externals/boost_1_46_1
			${CMAKE_CURRENT_SOURCE_DIR}/../../../externals/boost_1_46_1
            ${CMAKE_CURRENT_SOURCE_DIR}/../../tools/ToolsUtils
            )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

SET_UNICODE_OFF()

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} STATIC ${TARGET_SOURCES})
if (NOT WIN32)
  link_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../../externals/boost_1_46_1/lib/linux/${BOOST_LIB_DIR_SUFFIX})
  target_link_libraries( ${TARGET_NAME} cl_hw_utils numa )
else ()
  link_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../../externals/boost_1_46_1/lib/win/${BOOST_LIB_DIR_SUFFIX} )
  target_link_libraries( ${TARGET_NAME} cl_hw_utils )
endif(NOT WIN32)
 
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME} )

install( TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)

