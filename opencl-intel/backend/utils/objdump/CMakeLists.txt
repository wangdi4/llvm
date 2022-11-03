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

set(TARGET_NAME ObjDump)

# Copy and modify files from llvm-objdump dir to current build dir
set(ORIGINAL_OBJDUMP_DIR ${LLVM_MAIN_SRC_DIR}/tools/llvm-objdump)
file(GLOB ORIGINAL_OBJDUMP_FILES ${ORIGINAL_OBJDUMP_DIR}/*.h
     ${ORIGINAL_OBJDUMP_DIR}/*.cpp)
set(COPIED_OBJDUMP_FILES "")
foreach(ORIGINAL_OBJDUMP_FILE ${ORIGINAL_OBJDUMP_FILES})
  get_filename_component(BASENAME ${ORIGINAL_OBJDUMP_FILE} NAME)
  list(APPEND COPIED_OBJDUMP_FILES ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME})
endforeach()
add_custom_command(
  OUTPUT ${COPIED_OBJDUMP_FILES}
  COMMAND
    ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/redirect_objdump_to_file.py
    ${ORIGINAL_OBJDUMP_FILES} ${COPIED_OBJDUMP_FILES}
  DEPENDS ${ORIGINAL_OBJDUMP_FILES}
  DEPENDS llvm-objdump
  COMMENT "Generating OpenCL ObjDump files into ${CMAKE_CURRENT_BINARY_DIR}")

include_directories(
  ${LLVM_INCLUDE_DIRS}
  # path of tablegen generated include files.
  ${LLVM_OBJ_DIR}/tools/llvm-objdump)

add_opencl_library(${TARGET_NAME} STATIC ObjectDump.h ObjectDump.cpp
                   ${COPIED_OBJDUMP_FILES})

# For a tidy Visual Studio solution layout
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME})
