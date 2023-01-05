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
# INTEL_CUSTOMIZATION
foreach(flag_var
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
string(REGEX REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
endforeach()

if (MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "/MDd ${CMAKE_CXX_FLAGS_DEBUG}")
endif()

intel_clear_stdlib()
# end INTEL_CUSTOMIZATION
add_executable(sycl-ls sycl-ls.cpp)
add_dependencies(sycl-ls sycl)
target_include_directories(sycl-ls PRIVATE "${sycl_inc_dir}")

set(sycl_lib sycl)
string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type_lower)
if (WIN32 AND "${build_type_lower}" MATCHES "debug")
  set(sycl_lib sycld)
endif()

target_link_libraries(sycl-ls
  PRIVATE
    ${sycl_lib}
    OpenCL-Headers
)
install(TARGETS sycl-ls
  RUNTIME DESTINATION "bin" COMPONENT sycl-ls)
