# Copyright (C) 2011-2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express or
# implied warranties, other than those that are expressly stated in the License.

#
# Project variables: OCL_SOURCE_DIR    - points to the top src directory
# OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root
#                     of generated files
#

# set that name of the main output file as a target name
set(TARGET_NAME cl_logger)

# set emulator target name
# FIXME: that is a hack to build both CPU and FPGA emulator libs within a single
# building process. Better solutions:
#   - Merge _all_ CPU RT libraries into one;
#   - Make both CPU and FPGA emu platforms be accessible from a single binary
#     set.
set(TARGET_NAME_EMU ${TARGET_NAME}_${OUTPUT_EMU_SUFF})

include_directories(${CL_API_HEADERS})

find_sources()
calculate_target_sources()

include(itt)

if(WIN32)
  set(LIB_TYPE STATIC)
  set(SYSTEM_LIBS)
  set(TARGET_INSTALL_PATH)
else()
  # Linux
  set(LIB_TYPE SHARED)
  set(SYSTEM_LIBS rt)
  set(TARGET_INSTALL_PATH ${OUTPUT_ARCH_SUFF})
endif(WIN32)

# add a target named ${TARGET_NAME}
add_opencl_library(
  ${TARGET_NAME}
  ${LIB_TYPE}
  ${TARGET_SOURCES}
  LINK_LIBS
  cl_hw_utils
  cl_sys_utils
  ${SYSTEM_LIBS}
  INSTALL_PATH
  ${TARGET_INSTALL_PATH})

add_opencl_library(
  ${TARGET_NAME_EMU}
  ${LIB_TYPE}
  ${TARGET_SOURCES}
  LINK_LIBS
  cl_hw_utils
  cl_sys_utils
  ${SYSTEM_LIBS}
  INSTALL_PATH
  ${TARGET_INSTALL_PATH})

set_linux_exports_file(${TARGET_NAME} cl_logger_linux_exports.txt)
set_linux_exports_file(${TARGET_NAME_EMU} cl_logger_linux_exports.txt)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME})
set_target_properties(${TARGET_NAME_EMU} PROPERTIES FOLDER ${UTILS_FOLDER_NAME})
