set(TARGET_NAME OCLLoopIdiomRecognize)

set(TARGET_INCLUDE_FILES
    OCLLoopIdiomRecognize.h
)

set(TARGET_SOURCE_FILES
    OCLLoopIdiomRecognize.cpp
)

include_directories(
    ${CMAKE_BINARY_DIR}/include
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
