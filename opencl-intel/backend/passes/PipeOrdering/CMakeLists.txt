set(TARGET_NAME PipeOrdering)

set(TARGET_INCLUDE_FILES
    PipeOrdering.h
)

set(TARGET_SOURCE_FILES
    PipeOrdering.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/passes
    ${BACKEND_ROOT_DIR}/barrier/include
    ${BACKEND_ROOT_DIR}/utils
    ${OCL_SOURCE_DIR}/cl_api
    ${CMAKE_BINARY_DIR}/include/
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

add_dependencies(${TARGET_NAME} metadataapi)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
