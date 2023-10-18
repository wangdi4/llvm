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

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|IntelLLVM" )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-covered-switch-default")
  # INTEL_CUSTOMIZATION
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")
  # end INTEL_CUSTOMIZATION
endif()

# all plugins on unified_runtime plugin and unified_runtime plugin is not an
# independent plugin, adding it explicitly
add_subdirectory(unified_runtime)

foreach(plugin ${SYCL_ENABLE_PLUGINS})
  add_subdirectory(${plugin})
endforeach()
