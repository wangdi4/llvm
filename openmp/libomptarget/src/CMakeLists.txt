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
##===----------------------------------------------------------------------===##
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
##===----------------------------------------------------------------------===##
#
# Build offloading library libomptarget.so.
#
##===----------------------------------------------------------------------===##

libomptarget_say("Building offloading runtime library libomptarget.")

if(INTEL_CUSTOMIZATION)
  # We do not want to use version number in libomptarget, so we completely
  # diverge from upstream for now.
  set(LIBOMPTARGET_SRC_FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/api.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/device.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/interface.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/interop.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/omptarget.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/rtl.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/LegacyAPI.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/xpti_registry.cpp)
  if(SYCL_ENABLE_XPTI_TRACING)
    # Add XPTI proxy library
    list(APPEND LIBOMPTARGET_SRC_FILES
        ${LLVM_EXTERNAL_XPTI_SOURCE_DIR}/src/xpti_proxy.cpp)
  endif(SYCL_ENABLE_XPTI_TRACING)

  # Build libomptarget library with libdl dependency.
  add_library(omptarget SHARED ${LIBOMPTARGET_SRC_FILES})

  # Include/Definitions
  target_include_directories(omptarget PRIVATE
      ${LIBOMPTARGET_INCLUDE_DIR}
      ${LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER})
  if(SYCL_ENABLE_XPTI_TRACING)
    target_include_directories(omptarget PRIVATE
        ${LLVM_EXTERNAL_XPTI_SOURCE_DIR}/include)
    target_compile_definitions(omptarget PRIVATE
        XPTI_ENABLE_INSTRUMENTATION
        XPTI_STATIC_LIBRARY)
  endif(SYCL_ENABLE_XPTI_TRACING)

  add_dependencies(omptarget libomptarget-libiomp-file)

  target_link_libraries(omptarget PRIVATE
      ${CMAKE_DL_LIBS}
      ${LIBOMP_LIB_FILE}
      LLVMSupport
      LLVMObject)
  if(WIN32)
    libomptarget_add_resource_file(omptarget)
  else(WIN32)
    target_link_libraries(omptarget PRIVATE
        "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports")
  endif(WIN32)

  set_target_properties(omptarget PROPERTIES
      INSTALL_RPATH "$ORIGIN"
      BUILD_RPATH "$ORIGIN")

  set(OPENMP_ENABLE_LIBOMPTARGET_PROFILING OFF)
  if(OPENMP_ENABLE_LIBOMPTARGET_PROFILING)
    # Add LLVMSupport dependency if profiling is enabled.
    # Linking with LLVM component libraries also requires
    # aligning the compile flags.
    llvm_update_compile_flags(omptarget)
    target_compile_definitions(omptarget PUBLIC OMPTARGET_PROFILE_ENABLED)
    target_link_libraries(omptarget PRIVATE LLVMSupport)
  endif(OPENMP_ENABLE_LIBOMPTARGET_PROFILING)

  install(TARGETS omptarget LIBRARY COMPONENT omptarget
      DESTINATION "${OPENMP_INSTALL_LIBDIR}"
      RUNTIME COMPONENT omptarget DESTINATION bin
      ARCHIVE COMPONENT omptarget DESTINATION "${OPENMP_INSTALL_LIBDIR}")

  # llvm_update_compile_flags properly disables exceptions, so the system
  # headers (which may use EH, e.g MSVC's xlocale) can be properly compiled.
  llvm_update_compile_flags(omptarget)
  if(NOT LIBOMPTARGET_DEP_LIBELF_FOUND OR LIBOMPTARGET_FORCE_LLVM_ELF)
    # FIXME: we have to use the same C run-time flags (e.g. /MTd) as
    # the plguins, otherwise standard C++ types will have different
    # layouts in omptarget.dll and the plugins. We currently have
    # implicit/opaque C++ interface for ompt between libomptarget
    # and the plugins: CMPLRLIBS-33675
    set_msvc_crt_flags(omptarget)
  endif()

else(INTEL_CUSTOMIZATION)
add_llvm_library(omptarget
  SHARED

  api.cpp
  device.cpp
  interface.cpp
  interop.cpp
  omptarget.cpp
  rtl.cpp
  LegacyAPI.cpp

  ADDITIONAL_HEADER_DIRS
  ${LIBOMPTARGET_INCLUDE_DIR}

  LINK_COMPONENTS
  Support
  Object

  LINK_LIBS 
  PRIVATE
  ${CMAKE_DL_LIBS}
  "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports"
  NO_INSTALL_RPATH
)
target_include_directories(omptarget PRIVATE ${LIBOMPTARGET_INCLUDE_DIR})

# libomptarget.so needs to be aware of where the plugins live as they
# are now separated in the build directory.
set_target_properties(omptarget PROPERTIES INSTALL_RPATH "$ORIGIN" BUILD_RPATH "$ORIGIN:${CMAKE_CURRENT_BINARY_DIR}/..")
install(TARGETS omptarget LIBRARY COMPONENT omptarget DESTINATION "${OPENMP_INSTALL_LIBDIR}")
endif(INTEL_CUSTOMIZATION)
