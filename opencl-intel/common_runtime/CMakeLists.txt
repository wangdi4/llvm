#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

add_subdirectory( utils )
# set that name of the main output file as a target name
set( TARGET_NAME common_runtime )

include_directories( BEFORE
            ${CMAKE_CURRENT_SOURCE_DIR}/Utils
			${CMAKE_CURRENT_SOURCE_DIR}/../framework/icd
            )


set(FILES_TO_COPY 
		${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX} 
		)
			
FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

target_link_libraries( ${TARGET_NAME} common_runtime_utils)

set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME "intelopencl${ADDR}")

set ( FILES_TO_COPY "intelopencl${ADDR}${DLL_SUFIX}")

COPY_GENERATED_FILES_POST_BUILD( ${TARGET_NAME} ${FILES_TO_COPY} )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER common_runtime )
