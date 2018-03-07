set(TARGET_NAME AddImplicitArgs)

set(TARGET_INCLUDE_FILES
    AddImplicitArgs.h
)

set(TARGET_SOURCE_FILES
    AddImplicitArgs.cpp
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
