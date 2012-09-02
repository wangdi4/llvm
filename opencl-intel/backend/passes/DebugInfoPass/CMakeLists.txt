set(TARGET_NAME DebugInfoPass)

set(DEBUGINFOPASS_SOURCE_FILES
    DebugInfoPass.cpp
)

add_library(${TARGET_NAME}
    ${DEBUGINFOPASS_SOURCE_FILES}
)
