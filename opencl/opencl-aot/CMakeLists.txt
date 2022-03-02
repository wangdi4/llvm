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
cmake_minimum_required(VERSION 3.14)

set(LLVM_LINK_COMPONENTS
        ${LLVM_TARGETS_TO_BUILD}
        Support
        )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB TARGET_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

set(OPENCL_AOT_PROJECT_NAME opencl-aot)

# INTEL_CUSTOMIZATION
add_llvm_tool_in_bin(${OPENCL_AOT_PROJECT_NAME} ${TARGET_SOURCES})
# end INTEL_CUSTOMIZATION

# INTEL_CUSTOMIZATION
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # MSVC can't expand variadic macros correctly. Add compile flag to around this.
  target_compile_options(${OPENCL_AOT_PROJECT_NAME} PRIVATE "/experimental:preprocessor")
endif()
# end INTEL_CUSTOMIZATION

if(NOT MSVC)
  # FIXME: when built with clang it produces a warning.
  target_compile_options(${OPENCL_AOT_PROJECT_NAME} PRIVATE "-Wno-unused-parameter")
endif()

target_link_libraries(${OPENCL_AOT_PROJECT_NAME}
  PRIVATE
    OpenCL-Headers
    OpenCL-ICD)
