set(TARGET_NAME ModuleCleanup)

set(MODULECLEANUP_SOURCE_FILES
    ModuleCleanup.cpp
)

add_library(${TARGET_NAME}
    ${MODULECLEANUP_SOURCE_FILES}
)
