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

set(TARGET_NAME FMAtest)

# FusedMAtest has to use deprecated OpenCL API functions.
if(WIN32)
  add_definitions(-wd4996 # Suppress 'function': was declared deprecated'
  )
endif(WIN32)

include_directories(
  ${CL_API_HEADERS}
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/utils
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend
  ${BACKEND_ROOT_DIR}/common_runtime/icd
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
  ${OCL_SOURCE_DIR}/tests/test_utils/export)
add_opencl_executable(${TARGET_NAME} EXCLUDE_FROM_ALL_BUILD FusedMAtest.cpp)

if(NOT WIN32)
  set(LIBRARIES OclBackendUtils llvm_gtest pthread)
else(NOT WIN32)
  set(LIBRARIES OclBackendUtils llvm_gtest)
endif(NOT WIN32)

target_link_libraries(${TARGET_NAME} ${LIBRARIES} OpenCL-ICD)

install(
  TARGETS ${TARGET_NAME}
  RUNTIME DESTINATION tests/FMATest
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION tests/FMATest)
