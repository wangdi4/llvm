#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME task_executor )

include_directories( BEFORE . )

link_directories( ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR}
					)

add_definitions( -DTASK_EXECUTOR_EXPORTS -D__TBB_EXECUTOR__ -D_USRDLL )

SET_UNICODE_OFF()


# only tbb executor is supported on Linux
if (NOT WIN32)
    set( IGNORE_FILES thread_executor|xn_ )
endif(NOT WIN32)

if (WIN32)
     # tbb name is encoded in windows sources to use the right one
	 
	 # add GPA include files, libs and defines
	 include_directories(
						${CMAKE_CURRENT_SOURCE_DIR}/../../externals/gpa/include 
				    )
	link_directories(
					${CMAKE_CURRENT_SOURCE_DIR}/../../externals/gpa/libs/${TAL_LIB_DIR_SUFFIX} 
				)						
    add_definitions( -DUSE_GPA )
else ()
     if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
        list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb_debug${IMPLIB_SUFFIX}
                              ${IMPLIB_PREFIX}tbbmalloc_debug${IMPLIB_SUFFIX} )
     else ()
        list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb${IMPLIB_SUFFIX}
                              ${IMPLIB_PREFIX}tbbmalloc${IMPLIB_SUFFIX} )
     endif (${CMAKE_BUILD_TYPE} STREQUAL Debug)
endif (WIN32)

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

install(TARGETS ${TARGET_NAME} 
		RUNTIME DESTINATION bin
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION bin)

target_link_libraries( ${TARGET_NAME} ${LINK_LIBS} cl_logger cl_sys_utils )
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME} )


