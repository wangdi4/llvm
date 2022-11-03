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

# Find the native LLVM includes and library
#
# LLVM_LIBRARY_DIRS - where to find llvm libs LLVM_INCLUDE_DIRS - where to find
# llvm include files LLVM_MODULE_LIBS - list of llvm libs for working with
# modules.

if(BUILD_LLVM_FROM_SOURCE)
  set(LLVM_INCLUDE_DIRS
      ${LLVM_SRC_ROOT}/include
      ${LLVM_SRC_ROOT}/include/Intel_OptionalComponents
      ${LLVM_SRC_ROOT}/tools/clang/include
      ${CMAKE_CURRENT_BINARY_DIR}/llvm/include
      ${CMAKE_CURRENT_BINARY_DIR}/llvm/tools/clang/include)
  set(LLVM_LIBRARY_DIRS
      ${CMAKE_CURRENT_BINARY_DIR}/llvm/lib/${CMAKE_CFG_INTDIR})
  set(LLVM_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/llvm/bin/${CMAKE_CFG_INTDIR})
  set(LLVM_MODULE_LIBS ${STATIC_LLVM_MODULE_LIBS})
else(BUILD_LLVM_FROM_SOURCE)
  # reset the LLVM related variables
  set(LLVM_MODULE_LIBS)
  if(DEFINED CMAKE_MODULE_PATH_ORIGINAL)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH_ORIGINAL})
  else()
    set(CMAKE_MODULE_PATH_ORIGINAL ${CMAKE_MODULE_PATH})
  endif()

  # detect where to look for the LLVM installation
  if(DEFINED LLVM_PATH)
    set(LLVM_INSTALL_PATH ${LLVM_PATH})
  else()
    message(
      FATAL_ERROR
        "Can't find LLVM library. Please specify LLVM library location using LLVM_PATH parameter to CMAKE"
    )
  endif()

  # detect where the LLVMConfig should be found
  if(DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LLVM_PATH}/lib/cmake/llvm)
  else()
    # we need to include some file from LLVM share, the problem is that for VS
    # the LLVM_PATH is not full but contain the ${CMAKE_CFG_INTDIR} macro we
    # need to explicitly check for existence of either release or debug cmake
    # then
    string(REPLACE \${CMAKE_CFG_INTDIR} Debug LLVM_PATH_DEBUG "${LLVM_PATH}")
    string(REPLACE \${CMAKE_CFG_INTDIR} Release LLVM_PATH_RELEASE
                   "${LLVM_PATH}")
    set(CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH} ${LLVM_PATH_DEBUG}/lib/cmake/llvm
        ${LLVM_PATH_RELEASE}/lib/cmake/llvm)
  endif()
  set(CMAKE_MODULE_PATH_SAVE ${CMAKE_MODULE_PATH})
  include(LLVMConfig)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH_SAVE})

  if(DEFINED LLVM_PACKAGE_VERSION)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  else()
    message(FATAL_ERROR "Can't find LLVM in supplied path: ${LLVM_PATH}")
  endif()

  set(LLVM_INSTALL_PREFIX ${LLVM_PATH})
  set(LLVM_INCLUDE_DIRS ${LLVM_PATH}/include
                        ${LLVM_PATH}/include/Intel_OptionalComponents)
  set(CCLANG_DEV_INCLUDE_DIRS ${LLVM_PATH}/include/cclang)
  set(CLANG_OPENCL_HEADERS_DIR ${LLVM_PATH}/include/cclang)
  # LLVM_BINARY_DIR is set only if AddLLVM.cmake is used.
  set(LLVM_BINARY_DIR ${LLVM_INSTALL_PREFIX}/bin)

  llvm_map_components_to_libnames(LLVM_MODULE_LIBS all)

  # remove predefined unnecessary components
  list(
    REMOVE_ITEM
    LLVM_MODULE_LIBS
    llvm_gtest
    llvm_gtest_main
    profile_rt-static
    profile_rt-shared
    LTO
    LTO_static
    Remarks
    Remarks_static)
endif()

# kept as comment for the debug purposes. message( STATUS "LLVM libs:
# ${LLVM_MODULE_LIBS}")
