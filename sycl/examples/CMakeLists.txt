# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
# end INTEL_CUSTOMIZATION
if( INTEL_CUSTOMIZATION )
  # Skip building examples on xmain as add_sycl_executable does not
  # configure clang to use custom gcc toolchain so that clang
  # uses system installed gcc which may appear too outdated.
  message(STATUS "Skip building SYCL examples")
else()
  add_sycl_executable(simple-dpcpp-app SOURCES simple-dpcpp-app.cpp)
endif()
