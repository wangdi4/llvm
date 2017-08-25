set(TARGET_NAME PreventDivCrashes)

set(TARGET_INCLUDE_FILES
    PreventDivCrashes.h
)

set(TARGET_SOURCE_FILES
    PreventDivCrashes.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/passes
    ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo
    ${BACKEND_ROOT_DIR}/name_mangling
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
