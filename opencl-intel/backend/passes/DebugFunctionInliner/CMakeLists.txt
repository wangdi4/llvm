set(TARGET_NAME DebugFunctionInliner)

set(TARGET_SOURCE_FILES
    DebugFunctionInliner.cpp
)

add_library(${TARGET_NAME}
    ${TARGET_SOURCE_FILES}
)

include_directories(
    ${BACKEND_ROOT_DIR}/passes
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})

