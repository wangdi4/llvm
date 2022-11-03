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

set(TARGET_NAME OclPluginManager)

# ITT/VTune integration
add_definitions(-DUSE_ITT)

# disable "implicit-fallthrough", "self-assign" and "static-in-inline"(windows
# only) warnings in itt/ittnotify/ittnotify_static.c
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|IntelLLVM")
  set_source_files_properties(
    ${OCL_SOURCE_DIR}/externals/itt/ittnotify/ittnotify_static.c
    PROPERTIES
      COMPILE_FLAGS
      "-Wno-implicit-fallthrough -Wno-self-assign -Wno-static-in-inline")
endif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|IntelLLVM")

add_opencl_library(
  ${TARGET_NAME}
  STATIC
  plugin_manager.h
  plugin_manager.cpp
  source_file.h
  source_file.cpp
  compile_data.h
  compile_data.cpp
  ${OCL_SOURCE_DIR}/externals/itt/ittnotify/ittnotify_static.c
  INCLUDE_DIRS
  ${LLVM_INCLUDE_DIRS}
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
  ${BACKEND_ROOT_DIR}/dynamic_lib
  ${CL_API_HEADERS}
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
  ${OCL_SOURCE_DIR}/utils/cl_hw_utils/export
  ${OCL_SOURCE_DIR}/externals/itt/include)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER
                                                ${BACKEND_FOLDER_NAME}/plugins)
