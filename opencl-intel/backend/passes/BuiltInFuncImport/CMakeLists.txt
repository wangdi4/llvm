set(TARGET_NAME BuiltInFuncImport)

include_directories(
    ${BACKEND_ROOT_DIR}/passes
    )

set(BUILTINFUNCIMPORT_SOURCE_FILES
    BuiltInFuncImport.cpp
)

add_library(${TARGET_NAME}
    ${BUILTINFUNCIMPORT_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})