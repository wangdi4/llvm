set(TARGET_NAME LocalBuffAnalysis)

set(TARGET_INCLUDE_FILES
    LocalBuffAnalysis.h
)

set(TARGET_SOURCE_FILES
    LocalBuffAnalysis.cpp
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

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
