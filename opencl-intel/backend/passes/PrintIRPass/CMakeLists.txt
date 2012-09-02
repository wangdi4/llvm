set(TARGET_NAME PrintIRPass)

set(PRINTIRPASS_INCLUDE_FILES
    PrintIRPass.h
)

set(PRINTIRPASS_SOURCE_FILES
    PrintIRPass.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
)

add_library(${TARGET_NAME}
    ${PRINTIRPASS_INCLUDE_FILES}
    ${PRINTIRPASS_SOURCE_FILES}
)
