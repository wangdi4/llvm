set(TARGET_NAME SPIR20BlocksToObjCBlocks)

set(TARGET_INCLUDE_FILES
    SPIR20BlocksToObjCBlocks.h
)

set(TARGET_SOURCE_FILES
    SPIR20BlocksToObjCBlocks.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/passes
    ${BACKEND_ROOT_DIR}/utils
    ${CMAKE_BINARY_DIR}/include
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
