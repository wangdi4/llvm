set(TARGET_NAME InfiniteLoopCreator)

set(TARGET_INCLUDE_FILES
    InfiniteLoopCreator.h
)

set(TARGET_SOURCE_FILES
    InfiniteLoopCreator.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/passes
    ${CMAKE_SOURCE_DIR}/cl_api
    ${CMAKE_BINARY_DIR}/include
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

add_dependencies(${TARGET_NAME} metadataapi)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
