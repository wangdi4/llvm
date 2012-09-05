set(TARGET_NAME OclOptimizer)

set(ADDIMPLICITARGS_INCLUDE_FILES
    Optimizer.h
)

set(ADDIMPLICITARGS_SOURCE_FILES
    Optimizer.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/passes/PrintIRPass
    ${BACKEND_ROOT_DIR}/vectorizer/VolcanoWrapper
    ${BACKEND_ROOT_DIR}/../cl_api
)

add_library(${TARGET_NAME}
    ${ADDIMPLICITARGS_INCLUDE_FILES}
    ${ADDIMPLICITARGS_SOURCE_FILES}
)
