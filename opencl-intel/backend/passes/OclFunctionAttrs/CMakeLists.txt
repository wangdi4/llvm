set(TARGET_NAME OclFunctionAttrs)

set(TARGET_INCLUDE_FILES
    OclFunctionAttrs.h
)

set(TARGET_SOURCE_FILES
    OclFunctionAttrs.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/passes
    ${CL_API_HEADERS}
    ${BACKEND_ROOT_DIR}/utils
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
