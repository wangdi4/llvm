set(TARGET_NAME ChannelPipeTransformation)

set(TARGET_INCLUDE_FILES
    ChannelPipeTransformation.h
)

set(TARGET_SOURCE_FILES
    ChannelPipeTransformation.cpp
    ChannelPipeUtils.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo
)

add_opencl_library(${TARGET_NAME} STATIC
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

target_link_libraries(${TARGET_NAME} clbltpipeinfo)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
