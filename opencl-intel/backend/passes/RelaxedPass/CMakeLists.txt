set(TARGET_NAME RelaxedPass)

set(TARGET_INCLUDE_FILES
    RelaxedPass.h
)

set(TARGET_SOURCE_FILES
    RelaxedPass.cpp
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
