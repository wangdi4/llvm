#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME MemoryAllocator )

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

include(itt)

# add a target named ${TARGET_NAME}
add_opencl_library (${TARGET_NAME} STATIC ${TARGET_SOURCES})

target_link_libraries(${TARGET_NAME} clbltpipe)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME} )
