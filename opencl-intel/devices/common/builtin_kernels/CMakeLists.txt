#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set (TARGET_NAME builtin_kernels)
if (NOT WIN32)
    set(HW_UTILS_HEADERS ../../utils/cl_hw_utils/export)
    set(HW_UTILS_PROJ_NAME cl_hw_utils)
endif (NOT WIN32)

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

add_opencl_library (${TARGET_NAME} STATIC
    ${TARGET_SOURCES}

    INCLUDE_DIRS
    ${HW_UTILS_HEADERS}
    ${CL_API_HEADERS}
    ${OCL_SOURCE_DIR}/externals/itt/include

    INSTALL_PATH
    ${OUTPUT_ARCH_SUFF}
    )

set_target_properties (${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME})
