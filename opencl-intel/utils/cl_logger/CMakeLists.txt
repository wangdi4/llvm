#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME cl_logger )

# set emulator target name
# FIXME: that is a hack to build both CPU and FPGA emulator libs within a single
# building process. Better solutions:
# - Merge _all_ CPU RT libraries into one;
# - Make both CPU and FPGA emu platforms be accessible from a single binary set.
set( TARGET_NAME_EMU ${TARGET_NAME}_${OUTPUT_EMU_SUFF})

include_directories( ${CL_API_HEADERS} )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

include(itt)

if (WIN32)
    set( LIB_TYPE STATIC  )
    set( SYSTEM_LIBS      )
else()
    # Linux
    set( LIB_TYPE SHARED  )
    set( SYSTEM_LIBS rt )
endif(WIN32)

# add a target named ${TARGET_NAME}
add_opencl_library (${TARGET_NAME} ${LIB_TYPE}
    ${TARGET_SOURCES}

    LINK_LIBS
    cl_hw_utils
    cl_sys_utils
    ${SYSTEM_LIBS}

    INSTALL_PATH
    ${OUTPUT_ARCH_SUFF}
    )

add_opencl_library (${TARGET_NAME_EMU} ${LIB_TYPE}
    ${TARGET_SOURCES}

    LINK_LIBS
    cl_hw_utils
    cl_sys_utils
    ${SYSTEM_LIBS}

    INSTALL_PATH
    ${OUTPUT_ARCH_SUFF}
    )

SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} cl_logger_linux_exports.txt )
SET_LINUX_EXPORTS_FILE( ${TARGET_NAME_EMU} cl_logger_linux_exports.txt )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME} )
set_target_properties( ${TARGET_NAME_EMU} PROPERTIES FOLDER ${UTILS_FOLDER_NAME} )
