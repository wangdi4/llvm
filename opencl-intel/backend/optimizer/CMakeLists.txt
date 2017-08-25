set(TARGET_NAME OclOptimizer)

set(TARGET_INCLUDE_FILES
    Optimizer.h
)

set(TARGET_SOURCE_FILES
    Optimizer.cpp
)

include_directories(
    ${LLVM_INCLUDE_DIRS}
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend
    ${CL_API_HEADERS}
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/vectorizer/Utils
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/metadata_api
    ${CMAKE_BINARY_DIR}/include/
)

include_directories(
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/passes/PrintIRPass
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )

add_dependencies(${TARGET_NAME} metadataapi)
