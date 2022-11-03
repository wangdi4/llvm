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

if(WIN32)
  ocl_replace_compiler_option_to_dynamic()
endif(WIN32)
#
# Project variables: OCL_SOURCE_DIR    - points to the top src directory
# OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root
#                     of generated files
#

# set that name of the main output file as a target name
set(TARGET_NAME dyn_cl_sys_utils)

find_sources_in_folder(../cl_sys_utils)
calculate_target_sources()

set_unicode_off()

# Include the ASM sources
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  if(WIN32)
    create_asm_rules(UTILS_ASM_ADD_TO_SOURCES_LIST
                     ../cl_sys_utils/kernel_execute_64.asm)
  else(WIN32)
    set(UTILS_ASM_ADD_TO_SOURCES_LIST ../cl_sys_utils/linux64_utils.s)
  endif(WIN32)
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

include(itt)

# add a target named ${TARGET_NAME}
add_opencl_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCES}
  ${UTILS_ASM_ADD_TO_SOURCES_LIST}
  INCLUDE_DIRS
  ../cl_hw_utils/export
  ${CCLANG_DEV_INCLUDE_DIRS}
  ${CL_API_HEADERS}
  ${OCL_BINARY_DIR}
  ${OCL_SOURCE_DIR}/FrontendDriver
  INSTALL_PATH
  ${OUTPUT_ARCH_SUFF})

if(USE_NUMA AND NOT WIN32)
  add_definitions(-DUSE_NUMA=1)
  target_link_libraries(${TARGET_NAME} numa)
endif(USE_NUMA AND NOT WIN32)

# add Version.lib in Windows for GetModuleProductVersion()
if(WIN32)
  target_link_libraries(${TARGET_NAME} Version.lib dyn_cl_hw_utils)
else(WIN32)
  target_link_libraries(${TARGET_NAME} dyn_cl_hw_utils)
endif(WIN32)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME})

if(OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} ${COMMON_CLANG})
endif()
