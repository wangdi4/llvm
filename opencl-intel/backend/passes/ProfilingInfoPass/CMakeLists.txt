set(TARGET_NAME ProfilingInfoPass)

set(PROFILINGINFOPASS_SOURCE_FILES
    ProfilingInfoPass.cpp
)

add_library(${TARGET_NAME}
    ${PROFILINGINFOPASS_SOURCE_FILES}
)
