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
	set( SYSTEM_LIBS      )
elseif( ANDROID)
    # Android
	set( LIB_TYPE SHARED  )
	set(FILES_TO_COPY ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
	set( SYSTEM_LIBS      )
else()
    # Linux
	set( LIB_TYPE SHARED  )
	set(FILES_TO_COPY ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
	set( SYSTEM_LIBS rt )
endif(WIN32)

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} ${LIB_TYPE} ${TARGET_SOURCES})

if (FILES_TO_COPY)
	install(TARGETS ${TARGET_NAME} 
			RUNTIME DESTINATION bin
			ARCHIVE DESTINATION lib
			LIBRARY DESTINATION bin)
endif(FILES_TO_COPY)

if (NOT WIN32)
    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} cl_logger_linux_exports.txt )
endif (NOT WIN32)


target_link_libraries( ${TARGET_NAME} cl_hw_utils cl_sys_utils ${SYSTEM_LIBS})
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME} )
