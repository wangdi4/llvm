set(TARGET_NAME AddImplicitArgs)

set(ADDIMPLICITARGS_INCLUDE_FILES
    AddImplicitArgs.h
)

set(ADDIMPLICITARGS_SOURCE_FILES
    AddImplicitArgs.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/passes/LocalBuffAnalysis
    ${BACKEND_ROOT_DIR}/passes
)

add_library(${TARGET_NAME}
    ${ADDIMPLICITARGS_INCLUDE_FILES}
    ${ADDIMPLICITARGS_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
