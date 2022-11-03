#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2022 Intel Corporation
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
set(TARGET_NAME mkl_kernels)
if(NOT WIN32)
  set(HW_UTILS_HEADERS ../../utils/cl_hw_utils/export)
  set(HW_UTILS_PROJ_NAME cl_hw_utils)
endif(NOT WIN32)

add_definitions(-DMKL_ILP64)

get_directory_property(OUT_VAR LINK_DIRECTORIES)

find_sources()
calculate_target_sources()

# add a target named ${TARGET_NAME}
add_opencl_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCES}
  INCLUDE_DIRS
  ${MKL_DIR}/include
  ${HW_UTILS_HEADERS}
  ../builtin_kernels
  LINK_LIBS
  ${HW_UTILS_PROJ_NAME}
  cl_sys_utils
  COMPONENTS
  cl_logger${OPENCL_BINARY_POSTFIX})

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME})
