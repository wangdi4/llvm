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

include_directories(
  ${CL_API_HEADERS}
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/utils
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
  ${OCL_SOURCE_DIR}/tests/test_utils/export)

if(NOT WIN32)
  set(LINK_LIBRARIES pthread)
endif()

add_opencl_executable(${TARGET_NAME}
  EXCLUDE_FROM_ALL_BUILD
  FusedMAtest.cpp
  LINK_LIBS
  OclBackendUtils
  llvm_gtest
  OpenCL-ICD
  ${LINK_LIBRARIES})

install(
  TARGETS ${TARGET_NAME}
  RUNTIME DESTINATION tests/FMATest
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION tests/FMATest)
