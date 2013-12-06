set(TARGET_NAME SmartGVN)

include_directories(
    ${BACKEND_ROOT_DIR}/passes
)

set(TARGET_INCLUDE_FILES
    SmartGVN.h
)

set(TARGET_SOURCE_FILES
    SmartGVN.cpp
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
