set(TARGET_NAME BlockToFuncPtr)

include_directories(
    ${BACKEND_ROOT_DIR}/passes
    ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/../cl_api
)

set(TARGET_INCLUDE_FILES
  BlockToFuncPtrPass.h
)

set(TARGET_SOURCE_FILES
  BlockToFuncPtrPass.cpp
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
