set(TARGET_NAME PrintfArgumentsPromotion)

set(TARGET_INCLUDE_FILES
    PrintfArgumentsPromotion.h
)

set(TARGET_SOURCE_FILES
    PrintfArgumentsPromotion.cpp
)

include_directories(
    ${CMAKE_BINARY_DIR}/include
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
