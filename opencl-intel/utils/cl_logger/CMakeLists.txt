#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME cl_logger )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

if (WIN32)
	set( LIB_TYPE STATIC  )
else()
    # Linux
	set( LIB_TYPE SHARED  )
	set(FILES_TO_COPY ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
endif(WIN32)

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} ${LIB_TYPE} ${TARGET_SOURCES})

if (FILES_TO_COPY)
	install(TARGETS ${TARGET_NAME} 
			RUNTIME DESTINATION bin
			ARCHIVE DESTINATION lib
			LIBRARY DESTINATION bin)
endif(FILES_TO_COPY)

target_link_libraries( ${TARGET_NAME} cl_hw_utils )
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME} )
