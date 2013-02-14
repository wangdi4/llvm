if(WIN32)
    ocl_replace_compiler_option_to_dynamic()
endif(WIN32)
#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME dyn_googletest )

include_directories( ../gtest/include
                     ../gtest/)

if( ANDROID )
  add_definitions( -DGTEST_HAS_CLONE=0 )
endif( ANDROID )

add_definitions( -D_SCL_SECURE_NO_WARNINGS )

FIND_SOURCES_IN_FOLDER(../gtest/src)
CALCULATE_TARGET_SOURCES()

SET_UNICODE_ON()

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} STATIC ${TARGET_SOURCES})

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${EXTERNAL_FOLDER_NAME} )

