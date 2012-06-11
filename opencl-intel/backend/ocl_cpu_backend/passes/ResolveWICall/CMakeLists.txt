set(TARGET_NAME ResolveWICall)

set(RESOLVEWICALL_INCLUDE_FILES
    ResolveWICall.h
)

set(RESOLVEWICALL_SOURCE_FILES
    ResolveWICall.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend/utils
)

add_library(${TARGET_NAME}
    ${RESOLVEWICALL_INCLUDE_FILES}
    ${RESOLVEWICALL_SOURCE_FILES}
)

target_link_libraries(${TARGET_NAME}
    OclUtils
)

add_dependencies(${TARGET_NAME}
    OclUtils
)

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)