#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

if (NOT WIN32)
    set( CMAKE_CXX_FLAGS_RELEASE                "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
    set( CMAKE_C_FLAGS_RELEASE                  "${CMAKE_C_FLAGS_RELEASE}   -D_FORTIFY_SOURCE=2")
    add_definitions( "-Wno-pointer-arith -Wno-reorder" )
else (NOT WIN32)
# Common Runtime has to use deprecated OpenCL API functions.
    add_definitions(
        -wd4996 # Suppress 'function': was declared deprecated'
    )
endif(NOT WIN32)

set( TARGET_NAME "intelopencl${ADDR}${EXPERIMENTAL_POSTFIX}" )

if ( WIN32 AND ( DEFINED ENV{DXSDK_DIR} ) )
    add_definitions (-DCL_DX9_MEDIA_SHARING_INTEL_EXT)
endif()

add_subdirectory( utils )

include_directories( BEFORE
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CL_API_HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OCL_BINARY_DIR}
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


install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER common_runtime )

if (NOT WIN32)
    set_target_properties( ${TARGET_NAME} PROPERTIES LINK_FLAGS -Wl,-Bsymbolic )
endif (NOT WIN32)

if (WIN32)
    file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_NAME}.pdb PDB_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES
        LINK_FLAGS_DEBUG    "${LINK_FLAGS_DEBUG} /PDB:${PDB_NAME}"
        LINK_FLAGS_RELEASE  "${LINK_FLAGS_RELEASE} /DEBUG /PDB:${PDB_NAME}"
    )
    if (INSTALL_PDBS)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_NAME}.pdb DESTINATION bin)
    endif(INSTALL_PDBS)
endif(WIN32)