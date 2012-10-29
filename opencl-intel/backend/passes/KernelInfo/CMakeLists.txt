set(TARGET_NAME KernelInfo)

set(KERNELINFO_INCLUDE_FILES
    KernelInfoPass.h
)

set(KERNELINFO_SOURCE_FILES
    KernelInfoPass.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/utils
)

add_library(${TARGET_NAME}
    ${KERNELINFO_INCLUDE_FILES}
    ${KERNELINFO_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
