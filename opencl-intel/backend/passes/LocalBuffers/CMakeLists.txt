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
    ${BACKEND_ROOT_DIR}/passes
)

add_library(${TARGET_NAME}
    ${LOCALBUFFERS_INCLUDE_FILES}
    ${LOCALBUFFERS_SOURCE_FILES}
)

target_link_libraries(${TARGET_NAME}
    OclBackendUtils
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
