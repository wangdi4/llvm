set(TARGET_NAME LinearIdResolver)

set(TARGET_INCLUDE_FILES
    LinearIdResolver.h
)

set(TARGET_SOURCE_FILES
    LinearIdResolver.cpp
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
