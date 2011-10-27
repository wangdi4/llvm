#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME OpenCL )

if (DEFINED CMAKE_IMPORT_LIBRARY_PREFIX OR DEFINED CMAKE_IMPORT_LIBRARY_SUFFIX)
    set( IMPORT_LIB ${CMAKE_IMPORT_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_IMPORT_LIBRARY_SUFFIX} )
endif (DEFINED CMAKE_IMPORT_LIBRARY_PREFIX OR DEFINED CMAKE_IMPORT_LIBRARY_SUFFIX)

set(FILES_TO_COPY ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
                  ${IMPORT_LIB} )

#this flag is use in the main CMake in order to bypass error behavior by compiler gcc 4.1
remove_definitions( -Wno-non-virtual-dtor )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

install(TARGETS ${TARGET_NAME} 
		RUNTIME DESTINATION bin
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION bin)

target_link_libraries( ${TARGET_NAME} cl_sys_utils )
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${FRAMEWORK_FOLDER_NAME} )

if (NOT WIN32)
	target_link_libraries( ${TARGET_NAME} dl )
endif(NOT WIN32)
