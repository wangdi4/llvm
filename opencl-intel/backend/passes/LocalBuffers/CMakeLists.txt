set(TARGET_NAME LocalBuffers)

set(LOCALBUFFERS_INCLUDE_FILES
    LocalBuffers.h
)

set(LOCALBUFFERS_SOURCE_FILES
    LocalBuffers.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/passes/LocalBuffAnalysis
)

add_library(${TARGET_NAME}
    ${LOCALBUFFERS_INCLUDE_FILES}
    ${LOCALBUFFERS_SOURCE_FILES}
)
