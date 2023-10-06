# INTEL CONFIDENTIAL
#
# Copyright (C) 2022 Intel Corporation
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

# Compiler settings.
add_cflags("/GS")
if(NOT DEFINED CMAKE_C_COMPILER OR NOT DEFINED CMAKE_CXX_COMPILER)
  find_program(cc_path "icx" REQUIRED)
  set(CMAKE_C_COMPILER ${cc_path} CACHE FILEPATH "")
  set(CMAKE_CXX_COMPILER ${cc_path} CACHE FILEPATH "")
  add_cflags("/fp:precise")
endif()

# Windows linker settings.
find_program(link_path "lld-link" REQUIRED)
set(CMAKE_C_LINK_EXECUTABLE ${link_path} CACHE FILEPATH "")
set(CMAKE_CXX_LINK_EXECUTABLE ${link_path} CACHE FILEPATH "")
set(CMAKE_LINKER ${link_path} CACHE FILEPATH "")
string(PREPEND CMAKE_EXE_LINKER_FLAGS "/NXCompat /DynamicBase /HIGHENTROPYVA ")
string(PREPEND CMAKE_SHARED_LINKER_FLAGS "/NXCompat /DynamicBase /HIGHENTROPYVA ")

# Windows xdev
set(TARGET_OS "WINNT" CACHE STRING "")
set(HOST_OS "WINNT" CACHE STRING "")
set(TARGET_WINNT ON CACHE BOOL "")
set(HOST_WINNT ON CACHE BOOL "")
xarch(IL0_PREBUILT_COMPONENTS_PREFIX ${ARCHPROJ} efi2win ${ARCHVER} win_qa_release)

# Include the extra libraries for an xisa build (the test is weird to avoid
# triggering the feature filtering tool).
if("${LLVM_INTEL_FEATURES}" MATCHES ".*XISA.*")
  xarch(IL0_PREBUILT_LIBIRC_COMPONENTS_PREFIX ${ARCHPROJ} efi2win_avx3 ${ARCHVER} win_qa_release)
endif()


# Misc options
set(LLVM_USE_INTEL_JITEVENTS ON CACHE BOOL "")
