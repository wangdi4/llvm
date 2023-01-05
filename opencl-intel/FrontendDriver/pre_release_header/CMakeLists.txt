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

set(PRE_RELEASE_HEADERS_LIB pre_release_header)
set(LINUX_RESOURCE_LINKER_COMMAND header_linker)

set(PRE_RELEASE_HEADER
    "${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/opencl_cth_pre_release.h")
set(PRE_RELEASE_HEADER_CPP opencl_cth_pre_release.h.cpp)

add_custom_command(
  OUTPUT ${PRE_RELEASE_HEADER_CPP}
  DEPENDS ${PRE_RELEASE_HEADER} ${LINUX_RESOURCE_LINKER_COMMAND}
  COMMAND ${LINUX_RESOURCE_LINKER_COMMAND} ${PRE_RELEASE_HEADER}
          ${PRE_RELEASE_HEADER_CPP} "OPENCL_CTH_PRE_RELEASE_H"
  COMMENT "Packing ${PRE_RELEASE_HEADER}")

add_custom_target(opencl.cth.pre.release.target
                  DEPENDS ${PRE_RELEASE_HEADER_CPP})

list(APPEND CL_HEADERS_SRC ${PRE_RELEASE_HEADER_CPP})

add_library(${PRE_RELEASE_HEADERS_LIB} OBJECT ${CL_HEADERS_SRC})

add_dependencies(${PRE_RELEASE_HEADERS_LIB} opencl.cth.pre.release.target)
