set(TARGET_NAME CacheBinaryHandler)

add_opencl_library(${TARGET_NAME}
    cache_binary_handler.cpp
    elf_binary.cpp

    INCLUDE_DIRS
    ${CL_API_HEADERS}
    ${OCL_SOURCE_DIR}/utils/CLElfLib
    ${OCL_SOURCE_DIR}/cl_api

    INSTALL_PATH
    ${OUTPUT_ARCH_SUFF}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME})
