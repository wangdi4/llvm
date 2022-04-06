#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME Execution )

if(WIN32)
    check_cxx_compiler_flag("/QxSSSE3" CXX_SUPPORTS_QXSSSE3)
    if (CXX_SUPPORTS_QXSSSE3)
        # icx needs this
        set_source_files_properties(conversion_rules.cpp COMPILE_FLAGS "/QxSSSE3")
    endif()
endif()

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

include(itt)

# add a target named ${TARGET_NAME}
add_opencl_library (${TARGET_NAME} STATIC ${TARGET_SOURCES})
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME} )
