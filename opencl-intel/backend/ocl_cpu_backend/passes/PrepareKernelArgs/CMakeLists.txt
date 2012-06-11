set(TARGET_NAME PrepareKernelArgs)

set(PREPAREKERNELARGS_INCLUDE_FILES
    PrepareKernelArgs.h
)

set(PREPAREKERNELARGS_SOURCE_FILES
    PrepareKernelArgs.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend/utils
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend/passes/LocalBuffAnalysis
)

add_library(${TARGET_NAME}
    ${PREPAREKERNELARGS_INCLUDE_FILES}
    ${PREPAREKERNELARGS_SOURCE_FILES}
)

target_link_libraries(${TARGET_NAME}
    OclUtils
    LocalBuffAnalysis
)

add_dependencies(${TARGET_NAME}
    OclUtils
    LocalBuffAnalysis
)

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)