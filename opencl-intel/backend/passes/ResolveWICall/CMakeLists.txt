set(TARGET_NAME ResolveWICall)

set(TARGET_INCLUDE_FILES
    ResolveWICall.h
)

set(TARGET_SOURCE_FILES
    ResolveWICall.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/passes
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

add_dependencies(${TARGET_NAME} metadataapi)
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
