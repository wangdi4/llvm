set(TARGET_NAME ImplicitArgsAnalysis)

set(TARGET_INCLUDE_FILES
    ImplicitArgsAnalysis.h
)

set(TARGET_SOURCE_FILES
    ImplicitArgsAnalysis.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/passes
    ${BACKEND_ROOT_DIR}/utils
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})

