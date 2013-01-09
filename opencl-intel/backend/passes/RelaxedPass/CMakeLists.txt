set(TARGET_NAME RelaxedPass)

set(RELAXEDPASS_INCLUDE_FILES
    RelaxedPass.h
)

set(RELAXEDPASS_SOURCE_FILES
    RelaxedPass.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/passes
)

add_library(${TARGET_NAME}
    ${RELAXEDPASS_INCLUDE_FILES}
    ${RELAXEDPASS_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
