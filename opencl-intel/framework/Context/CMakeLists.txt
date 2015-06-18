#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME Context )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

include_directories( 
            ${LLVM_INCLUDE_DIRS}/cclang/cl_headers
            ${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export
            ${CMAKE_SOURCE_DIR}/utils/CLElfLib 
            ${LLVM_INCLUDE_DIRS}/cclang
            )

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} STATIC ${TARGET_SOURCES})
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME} )
