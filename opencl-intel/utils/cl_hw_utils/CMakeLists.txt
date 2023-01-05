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

#
# Project variables: OCL_SOURCE_DIR    - points to the top src directory
# OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root
#                     of generated files
#

# set that name of the main output file as a target name
set(TARGET_NAME cl_hw_utils)

if(WIN32)
  if(BUILD_X64)
    file(
      GLOB ASM_SOURCES
      RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
      win32Imp/bin64/*.asm)
  else()
    file(
      GLOB ASM_SOURCES
      RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
      win32Imp/bin32/*.asm)
  endif()

  create_asm_rules(ASM_ADD_TO_SOURCES_LIST ${ASM_SOURCES})

else()
  # Linux
  file(
    GLOB ASM_ADD_TO_SOURCES_LIST
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    linuxImp/*.cpp)
endif(WIN32)

find_sources()
calculate_target_sources()

# add a target named ${TARGET_NAME}
add_opencl_library(${TARGET_NAME} STATIC ${TARGET_SOURCES}
                   ${ASM_ADD_TO_SOURCES_LIST})

set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE C)
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME})
