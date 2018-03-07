set(TARGET_NAME InternalizeGlobalVariables)

set(TARGET_SOURCE_FILES
    InternalizeGlobalVariables.cpp
)

add_library(${TARGET_NAME}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
