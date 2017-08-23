set(TARGET_NAME InstToFuncCall)

set(TARGET_INCLUDE_FILES
    InstToFuncCall.h
)

set(TARGET_SOURCE_FILES
    InstToFuncCall.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/passes
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
