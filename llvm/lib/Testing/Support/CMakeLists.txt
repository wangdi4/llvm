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
# Do not build unittest libraries automatically, they will be pulled in
# by unittests if these are built.
if (NOT ${LLVM_INSTALL_GTEST})
  set (BUILDTREE_ONLY BUILDTREE_ONLY)
  set(EXCLUDE_FROM_ALL ON)
endif()

add_llvm_library(LLVMTestingSupport
  Error.cpp
  SupportHelpers.cpp

  ${BUILDTREE_ONLY}

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Testing/Support

  LINK_COMPONENTS
  Support
  )

# INTEL_CUSTOMIZATION
if (LLVM_LIBCXX_USED AND NOT SYCL_USE_LIBCXX)
  add_llvm_library(LLVMTestingSupport_stdcpp
    Error.cpp
    SupportHelpers.cpp

    BUILDTREE_ONLY

    ADDITIONAL_HEADER_DIRS
    ${LLVM_MAIN_INCLUDE_DIR}/llvm/Testing/Support

    LINK_COMPONENTS
    Support_stdcpp

    STDLIB
    stdc++
  )
  target_link_libraries(LLVMTestingSupport_stdcpp PRIVATE llvm_gtest_stdcpp)
endif()
if (WIN32)
  add_llvm_library(LLVMTestingSupport_dyn
    Error.cpp
    SupportHelpers.cpp

    BUILDTREE_ONLY

    ADDITIONAL_HEADER_DIRS
    ${LLVM_MAIN_INCLUDE_DIR}/llvm/Testing/Support

    LINK_COMPONENTS
    Support_dyn
  )
  set_property(TARGET LLVMTestingSupport_dyn
               PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  target_link_libraries(LLVMTestingSupport_dyn PRIVATE llvm_gtest_dyn)
endif()
# end INTEL_CUSTOMIZATION
target_link_libraries(LLVMTestingSupport PRIVATE llvm_gtest)

# This is to avoid the error in gtest-death-test-internal.h
# (150,16): error: 'Create' overrides a member function but
# is not marked 'override' [-Werror,-Wsuggest-override]
# during self-compile on Windows.

if (HOST_WINNT AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|IntelLLVM" ) # INTEL
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-suggest-override")
endif()
