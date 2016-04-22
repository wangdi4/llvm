set(TARGET_NAME BuiltInAttributesImport)

include_directories(
    ${BACKEND_ROOT_DIR}/passes
    ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo
)

set(TARGET_INCLUDE_FILES
    BuiltInAttributesImport.h
)

set(TARGET_SOURCE_FILES
    BuiltInAttributesImport.cpp
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
