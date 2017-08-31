set(TARGET_NAME AutorunReplicator)

set(TARGET_INCLUDE_FILES
    AutorunReplicator.h
)

set(TARGET_SOURCE_FILES
    AutorunReplicator.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/passes
    ${BACKEND_ROOT_DIR}/utils
    ${CMAKE_SOURCE_DIR}/cl_api
    ${CMAKE_BINARY_DIR}/include
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

add_dependencies(${TARGET_NAME} metadataapi)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
