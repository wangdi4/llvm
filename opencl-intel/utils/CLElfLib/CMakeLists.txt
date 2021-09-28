set(TARGET_NAME CLElfLib)

add_opencl_library(${TARGET_NAME}
    ElfReader.cpp
    ElfWriter.cpp

    INCLUDE_DIRS
    ../cl_sys_utils/export

    INSTALL_PATH
    ${OUTPUT_ARCH_SUFF}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME})
