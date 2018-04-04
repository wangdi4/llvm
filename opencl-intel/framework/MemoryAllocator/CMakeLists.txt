#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME MemoryAllocator )

# [LLVM 3.6 UPGRADE] Disable Linux warning about unused function
# this warning occur when processing externals/itt/include/ittnotify.h
# since it contain __itt_id __itt_id_make(void*, long long unsigned int)
# function that is not used in every build target.
# The warning conversion-null occur when ccomparing SharedPtr with NULL.
if (NOT WIN32)
       add_definitions( "-Wno-unused-function -Wno-conversion-null" )
endif(NOT WIN32)

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} STATIC ${TARGET_SOURCES})

target_link_libraries(${TARGET_NAME} clbltpipe)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME} )
