set(TARGET_NAME RelaxedPass)

set(RELAXEDPASS_SOURCE_FILES
    RelaxedPass.cpp
)

add_library(${TARGET_NAME}
    ${RELAXEDPASS_SOURCE_FILES}
)
