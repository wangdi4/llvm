set(TARGET_NAME CacheBinaryHandler)

add_opencl_library(${TARGET_NAME}
    cache_binary_handler.cpp
    elf_binary.cpp

    INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/utils/CLElfLib
    ${CMAKE_SOURCE_DIR}/cl_api
    ${CL_API_HEADERS}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME})
