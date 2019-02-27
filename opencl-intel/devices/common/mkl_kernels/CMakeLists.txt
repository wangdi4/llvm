#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME mkl_kernels )
if (NOT WIN32)
    set(HW_UTILS_HEADERS ../../utils/cl_hw_utils/export)
    set(HW_UTILS_PROJ_NAME cl_hw_utils)
endif (NOT WIN32)

add_definitions(-DMKL_ILP64)

get_directory_property(OUT_VAR LINK_DIRECTORIES)

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

#add a target named ${TARGET_NAME}
add_opencl_library (${TARGET_NAME} STATIC
    ${TARGET_SOURCES}

    INCLUDE_DIRS
      ${MKL_DIR}/include
      ${HW_UTILS_HEADERS}
      ../builtin_kernels

    LINK_LIBS
      ${HW_UTILS_PROJ_NAME}
      cl_sys_utils

    COMPONENTS
      cl_logger${OPENCL_BINARY_POSTFIX}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME} )
