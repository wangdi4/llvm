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

include(FetchContent)

# Suppress warnings related to dependencies
if (MSVC)
  add_definitions(
    -wd4133 # Suppress ''function': incompatible types - from 'char [9]' to 'LPCWSTR''
    -wd4152 # Suppress 'nonstandard extension, function/data pointer conversion in expression'
    -wd4201 # Suppress 'nonstandard extension used: nameless struct/union'
  )
endif()

# Repo URLs

if(INTEL_CUSTOMIZATION)
  if(DEFINED ENV{ICS_GIT_MIRROR} AND NOT "$ENV{ICS_GIT_MIRROR}" STREQUAL "")
    STRING(REGEX REPLACE "\\\\" "/" GITSERVER "$ENV{ICS_GIT_MIRROR}")
  else()
    set(GITSERVER "https://github.com/intel-innersource")
  endif()
 set(OCL_HEADERS_REPO
   "${GITSERVER}/applications.compilers.source.opencl-headers")
 set(OCL_LOADER_REPO
   "${GITSERVER}/applications.compilers.source.opencl-icd-loader")
endif(INTEL_CUSTOMIZATION)

# Repo tags/hashes

set(OCL_HEADERS_TAG dcd5bede6859d26833cd85f0d6bbcee7382dc9b3)
set(OCL_LOADER_TAG 5d9177ee79bfbcc75ee9a8cff6415eab2c3113f6)

# OpenCL Headers
if(NOT OpenCL_HEADERS)
  message(STATUS "Will fetch OpenCL headers from ${OCL_HEADERS_REPO}")

  FetchContent_Declare(ocl-headers
    GIT_REPOSITORY    ${OCL_HEADERS_REPO}
    GIT_TAG           ${OCL_HEADERS_TAG}
  )
else()
  message(STATUS "OpenCL headers are added manually ${OpenCL_HEADERS}")

  FetchContent_Declare(ocl-headers
    URL               ${OpenCL_HEADERS}
  )
endif()

FetchContent_MakeAvailable(ocl-headers)
FetchContent_GetProperties(ocl-headers)
set(OpenCL_INCLUDE_DIR
  ${ocl-headers_SOURCE_DIR} CACHE PATH "Path to OpenCL Headers")

target_compile_definitions(Headers INTERFACE -DCL_TARGET_OPENCL_VERSION=220)
add_library(OpenCL-Headers ALIAS Headers)

# OpenCL Library (ICD Loader)

# Set OPENCL_ICD_LOADER_HEADERS_DIR, as prerequisite for ICD build
set(OPENCL_ICD_LOADER_HEADERS_DIR
  ${OpenCL_INCLUDE_DIR} CACHE PATH "Path to OpenCL Headers")

# LLVM build sets this OFF by default, but we need OpenCL to be built as shared
# library.
set(BUILD_SHARED_LIBS ON)

if(NOT OpenCL_LIBRARY_SRC)
  message(STATUS "Will fetch OpenCL ICD Loader from ${OCL_LOADER_REPO}")

  FetchContent_Declare(ocl-icd
    GIT_REPOSITORY    ${OCL_LOADER_REPO}
    GIT_TAG           ${OCL_LOADER_TAG}
  )
else()
  # TODO: add possibility to use prebuilt OpenCL library rather than building
  #       together with llvm.
  message(STATUS
    "OpenCL ICD Loader sources added manually ${OpenCL_LIBRARY_SRC}")

  FetchContent_Declare(ocl-icd
    URL               ${OpenCL_LIBRARY_SRC}
  )
endif()

FetchContent_MakeAvailable(ocl-icd)

add_library(OpenCL-ICD ALIAS OpenCL)

if(INTEL_CUSTOMIZATION)
  # OpenCL RT build requires headers from ICD loader, propagating location
  set(OpenCL_LOADER_DIR
    ${ocl-icd_SOURCE_DIR} CACHE PATH "Path to OpenCL Loader")

  # TODO: Remove suppressing Werror.
  if(MSVC)
    target_compile_options(OpenCL PRIVATE "-WX-")
  else()
    target_compile_options(OpenCL PRIVATE "-Wno-error")
  endif()
  # Deploy OpenCL loader into package, this is required for xmain tc tests.
  # The deploy destination is different for OpenCL stand-alone product. See
  # opencl-intel/framework/CMakeLists.txt.
  if (NOT INTEL_OPENCL_STANDALONE_PRODUCT)
    install(TARGETS OpenCL
            ARCHIVE DESTINATION "lib${LLVM_LIBDIR_SUFFIX}" COMPONENT OpenCL-ICD
            LIBRARY DESTINATION "lib${LLVM_LIBDIR_SUFFIX}" COMPONENT OpenCL-ICD
            RUNTIME DESTINATION "bin" COMPONENT OpenCL-ICD)
  endif()
endif(INTEL_CUSTOMIZATION)

add_subdirectory(opencl-aot)
