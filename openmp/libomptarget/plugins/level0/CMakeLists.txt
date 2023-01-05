# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2021 Intel Corporation
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
# INTEL_COLLAB
##===----------------------------------------------------------------------===##
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
##===----------------------------------------------------------------------===##
#
# Plugin for Level0 devices
#
##===----------------------------------------------------------------------===##

if(LIBOMPTARGET_DEP_LEVEL0_FOUND)
  if((CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64)$" OR
      CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64") AND
     (CMAKE_SYSTEM_NAME MATCHES "Linux" OR
      CMAKE_SYSTEM_NAME MATCHES "Windows"))

    libomptarget_say("Building Level0 offloading plugin.")

    # Define the suffix for the runtime messaging dumps.
    #add_definitions(-DTARGET_NAME=LEVEL0)
    #add_definitions(-DTARGET_LEVEL0_VERSION=?)

    if(LIBOMPTARGET_CMAKE_BUILD_TYPE MATCHES debug)
      add_definitions(-DOMPTARGET_LEVEL0_DEBUG)
    endif()

    if(INTEL_CUSTOMIZATION)
    add_library(omptarget.rtl.level0 SHARED src/rtl.cpp src/ompt-level0.cpp)
    else(INTEL_CUSTOMIZATION)
    add_library(omptarget.rtl.level0 SHARED src/rtl.cpp)
    endif(INTEL_CUSTOMIZATION)
    target_include_directories(omptarget.rtl.level0 PRIVATE
      ${LIBOMPTARGET_INCLUDE_DIR}
      ${LIBOMPTARGET_DEP_LEVEL0_INCLUDE_DIRS}
      ${LIBOMPTARGET_LLVM_INCLUDE_DIRS}
    )

    if(INTEL_CUSTOMIZATION)
      add_library(omptarget.sycl.wrap SHARED src/omp_sycl.cpp)
      add_dependencies(omptarget.sycl.wrap sycl)
      target_include_directories(omptarget.sycl.wrap PRIVATE
          ${LIBOMPTARGET_INCLUDE_DIR}
          ${LIBOMPTARGET_DEP_LEVEL0_INCLUDE_DIRS}
          ${LLVM_BINARY_DIR}/include/sycl)
      # Need to silence deprecated-declarations for the sycl wrapper
      check_cxx_compiler_flag(-Wno-deprecated-declarations
                              LIBOMPTARGET_HAVE_WNO_DEPRECATED_DECLARATIONS)
      if(LIBOMPTARGET_HAVE_WNO_DEPRECATED_DECLARATIONS)
        target_compile_options(omptarget.sycl.wrap PRIVATE
                               -Wno-deprecated-declarations)
      endif()
      set_property(TARGET omptarget.sycl.wrap PROPERTY CXX_STANDARD 17)

      if(NOT LIBOMPTARGET_DEP_LIBELF_FOUND OR LIBOMPTARGET_FORCE_LLVM_ELF)
        # Statically link C run-time library on Windows.
        # Otherwise, we cannot use LLVMObject and LLVMSupport.
        set_msvc_crt_flags(omptarget.rtl.level0)
      endif()
    endif(INTEL_CUSTOMIZATION)

    if(NOT OPENMP_STANDALONE_BUILD)
      # We have to disable EH for Windows compilation.
      # For standalone OpenMP build, we need to come up
      # with our own EH flags management.
      llvm_update_compile_flags(omptarget.rtl.level0)
      if(INTEL_CUSTOMIZATION)
        llvm_update_compile_flags(omptarget.sycl.wrap)
      endif(INTEL_CUSTOMIZATION)
    endif()

    if(INTEL_CUSTOMIZATION)
    # Install plugin under the lib destination folder.
    # DLL part (for DLL platforms) is installed into bin.
    # Do not install interface library for DLL platforms,
    # since the plugin is only supposed to be used by libomptarget.
    install(TARGETS omptarget.rtl.level0 omptarget.sycl.wrap
            LIBRARY COMPONENT llvm-level0-ompplugin
            DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX}
            RUNTIME COMPONENT llvm-level0-ompplugin
            DESTINATION bin)

    add_dependencies(omptarget.rtl.level0 libomptarget-libiomp-file)
    else(INTEL_CUSTOMIZATION)
    install(TARGETS omptarget.rtl.level0
            LIBRARY DESTINATION "${OPENMP_INSTALL_LIBDIR}")
    endif(INTEL_CUSTOMIZATION)

    target_link_libraries(omptarget.rtl.level0 PRIVATE
      elf_common
      LLVMSupport)

    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
      target_link_libraries(omptarget.rtl.level0 PRIVATE
          ${LIBOMPTARGET_DEP_LEVEL0_LIBRARIES} ${LIBOMP_LIB_FILE} dl
          ${OPENMP_PTHREAD_LIB}
          "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports")
# INTEL_CUSTOMIZATION
      # Compilation of the sycl wrapper requires runtime type information and
      # exceptions enabled
      # Xmain's self-build phase forces -stdlib=libc++ but sycl library removes
      # this due to STL compatibility issues, and we need to force libstdc++ in
      # that case.
      if (LLVM_LIBCXX_USED AND NOT SYCL_USE_LIBCXX)
        set(LIBOMPTARGET_LEVEL0_SYCL_FLAG "-stdlib=libstdc++")
      endif()
      target_compile_options(omptarget.sycl.wrap PRIVATE
          -frtti -fexceptions
          ${LIBOMPTARGET_LEVEL0_SYCL_FLAG}
          -DSYCL_DISABLE_CPP_VERSION_CHECK_WARNING)
      target_link_libraries(omptarget.sycl.wrap PRIVATE sycl
          ${LIBOMPTARGET_LEVEL0_SYCL_FLAG})
    elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
      # Full path to the L0 library is recognized as a linker option, so we
      # separate directory and file name
      get_filename_component(LEVEL0_LIBRARY_PATH
          ${LIBOMPTARGET_DEP_LEVEL0_LIBRARIES} DIRECTORY)
      get_filename_component(LEVEL0_LIBRARY_NAME
          ${LIBOMPTARGET_DEP_LEVEL0_LIBRARIES} NAME)
      target_link_libraries(omptarget.rtl.level0 PRIVATE
          ${LEVEL0_LIBRARY_NAME} ${LIBOMP_LIB_FILE}
          -LIBPATH:${LEVEL0_LIBRARY_PATH})
      # Compilation of the sycl wrapper requires runtime type information and
      # exceptions enabled
      target_compile_options(omptarget.sycl.wrap PRIVATE
          -GR -EHsc -MD -DSYCL_DISABLE_CPP_VERSION_CHECK_WARNING)
      # We cannot provide the behavior how sycl and sycld is choosed since the
      # wrapper is dynamically loaded (LoadLibrary). Fixing it to sycl is the
      # best we can do now.
      target_link_libraries(omptarget.sycl.wrap PRIVATE sycl)
      libomptarget_add_resource_file(omptarget.rtl.level0)
      libomptarget_add_resource_file(omptarget.sycl.wrap)
# end INTEL_CUSTOMIZATION
    else()
      message(FATAL_ERROR "Missing platfrom support.")
    endif()

    # Report to the parent scope that we are building a plugin for Level0.
    set(LIBOMPTARGET_SYSTEM_TARGETS
      "${LIBOMPTARGET_SYSTEM_TARGETS} spir64-unknown-unknown" PARENT_SCOPE)
  else()
    libomptarget_say("Not building Level0 offloading plugin: only support Level0 in x86_64 Linux/Windows hosts.")
  endif()
else()
  libomptarget_say("Not building Level0 offloading plugin: Level0 not found in system.")
endif()

# end INTEL_COLLAB
