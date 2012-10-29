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
)

add_library(${TARGET_NAME}
    ${ADDIMPLICITARGS_INCLUDE_FILES}
    ${ADDIMPLICITARGS_SOURCE_FILES}
)

target_link_libraries(${TARGET_NAME}
    OclUtils
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
