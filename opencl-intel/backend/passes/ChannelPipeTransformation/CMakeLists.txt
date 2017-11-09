set(TARGET_NAME ChannelPipeTransformation)

set(TARGET_INCLUDE_FILES
    ChannelPipeTransformation.h
)

set(TARGET_SOURCE_FILES
    ChannelPipeTransformation.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/passes
    ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/../cl_api
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

target_link_libraries(${TARGET_NAME} clbltpipe)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
