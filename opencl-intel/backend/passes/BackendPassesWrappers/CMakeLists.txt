set(TARGET_NAME BackendPassesWrappers)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/passes
    ${BACKEND_ROOT_DIR}/passes/AddImplicitArgs
    ${BACKEND_ROOT_DIR}/passes/LocalBuffAnalysis
    ${BACKEND_ROOT_DIR}/passes/LocalBuffers
    ${BACKEND_ROOT_DIR}/passes/ModuleCleanup
)

set (PASSESWRAPPERS_SOURCE_FILES
    PassesWrappers.cpp
)

add_library(${TARGET_NAME}
    ${PASSESWRAPPERS_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "Backend" )
