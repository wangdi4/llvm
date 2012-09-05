set(TARGET_NAME ResolveWICall)

set(RESOLVEWICALL_INCLUDE_FILES
    ResolveWICall.h
)

set(RESOLVEWICALL_SOURCE_FILES
    ResolveWICall.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/utils
)

add_library(${TARGET_NAME}
    ${RESOLVEWICALL_INCLUDE_FILES}
    ${RESOLVEWICALL_SOURCE_FILES}
)
