set(TARGET_NAME StripIntelIP)

set(TARGET_INCLUDE_FILES
    StripIntelIP.h
)

set(TARGET_SOURCE_FILES
    StripIntelIP.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/metadata_api
    ${CMAKE_BINARY_DIR}/include
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
