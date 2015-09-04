#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

if (NOT WIN32)
    add_definitions( "-Wno-pointer-arith -Wno-reorder" )
endif(NOT WIN32)

add_subdirectory( utils )
# set that name of the main output file as a target name
set( TARGET_NAME common_runtime )

include_directories( BEFORE
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OCL_BINARY_DIR}
        )

if ( WIN32 AND ( DEFINED ENV{DXSDK_DIR} ) )
    add_definitions (-DCL_DX9_MEDIA_SHARING_INTEL_EXT)
endif()

set(FILES_TO_COPY
		${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
		)

if (WIN32)
    set( CONFIG_FILES register_vendor32.reg register_vendor64.reg unregister_vendor32.reg unregister_vendor64.reg)
endif (WIN32)

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

install( FILES ${CONFIG_FILES} DESTINATION bin)

if (NOT WIN32 AND NOT ANDROID)
    target_link_libraries( ${TARGET_NAME} pthread)
endif (NOT WIN32 AND NOT ANDROID)
target_link_libraries( ${TARGET_NAME} common_runtime_utils)

set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME "intelopencl${ADDR}")


install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER common_runtime )

if (NOT WIN32)
    set_target_properties( ${TARGET_NAME} PROPERTIES LINK_FLAGS -Wl,-Bsymbolic )
endif (NOT WIN32)
