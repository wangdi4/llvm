# //===----------------------------------------------------------------------===//
# //
# // Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# // See https://llvm.org/LICENSE.txt for details.
# // SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# //
# //===----------------------------------------------------------------------===//
# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2023 Intel Corporation
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

set(LIBOMP_ARCHER_SUPPORT TRUE CACHE BOOL "Build libomp with archer support")

if(OPENMP_ENABLE_ARCHER AND LIBOMP_ARCHER_SUPPORT) # INTEL
  include_directories(${LIBOMP_INCLUDE_DIR})

  add_library(archer SHARED ompt-tsan.cpp)
  target_link_libraries(archer ${CMAKE_DL_LIBS} "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports") # INTEL
  add_library(archer_static STATIC ompt-tsan.cpp)

  install(TARGETS archer archer_static
    COMPONENT archer # INTEL
    LIBRARY DESTINATION ${OPENMP_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${OPENMP_INSTALL_LIBDIR})

  add_subdirectory(tests)
endif()
