set(TARGET_NAME Prefetch)

set(PREFETCH_INCLUDE_FILES
    Prefetch.h
)

set(PREFETCH_SOURCE_FILES
    Prefetch.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/passes
)

add_library(${TARGET_NAME}
    ${PREFETCH_INCLUDE_FILES}
    ${PREFETCH_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
