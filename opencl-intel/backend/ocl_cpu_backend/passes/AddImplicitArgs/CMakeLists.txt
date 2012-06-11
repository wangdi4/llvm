set(TARGET_NAME AddImplicitArgs)

set(ADDIMPLICITARGS_INCLUDE_FILES
    AddImplicitArgs.h
)

set(ADDIMPLICITARGS_SOURCE_FILES
    AddImplicitArgs.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend/utils
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend/passes/LocalBuffAnalysis
)

add_library(${TARGET_NAME}
    ${ADDIMPLICITARGS_INCLUDE_FILES}
    ${ADDIMPLICITARGS_SOURCE_FILES}
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