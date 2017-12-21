set(TARGET_NAME InternalizeNonKernelFunc)

include_directories(
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/passes
    ${BACKEND_ROOT_DIR}/metadata_api
)

set(TARGET_SOURCE_FILES
    InternalizeNonKernelFunc.cpp
)

add_library(${TARGET_NAME}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
