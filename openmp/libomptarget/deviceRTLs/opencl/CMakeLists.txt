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
# Build the OpenCL device RTL if the OpenCL SDK is available
#
##===----------------------------------------------------------------------===##

if(LIBOMPTARGET_DEP_OPENCL_FOUND)
  libomptarget_say("Building OpenCL offloading device RTL.")

  set(opencl_inc
    src/internal.h
    src/omptarget-opencl.h
  )

  # Required part that should not be optimized away with --only-needed
  set(opencl_src_required src/omptarget-opencl.cl)

  # Optional part that can be optimized away with --only-needed
  set(opencl_src_optional
    src/abi.cl
    src/abi-atomic.cl
    src/abi-loop.cl
    src/abi-parallel.cl
    src/api.cl
    src/abi-atomic-fp.cpp
  )

  set(opencl_src ${opencl_src_optional} ${opencl_src_required})

  if (WIN32)
    set(objext .obj)
    set(host_target_name host-x86_64-pc-windows-msvc)
  else()
    set(objext .o)
    set(host_target_name host-x86_64-unknown-linux-gnu)
  endif()
  set(opencl_flags_spirv
    -cc1 -emit-llvm-bc -triple spir64-unknown-unknown -cl-std=CL2.0
    -disable-llvm-passes -include opencl-c.h -DINTEL_COLLAB=1
  )
  set(omp_cpp_flags -DINTEL_COLLAB=1 -c -fiopenmp -fopenmp-targets=spir64)

  if("$ENV{LIBOMPTARGET_NO_DOUBLE_PRECISION}" STREQUAL "1")
    list(APPEND opencl_flags_spirv -DHAVE_FP64_SUPPORT=0)
  else()
    list(APPEND opencl_flags_spirv -DHAVE_FP64_SUPPORT=1)
  endif()
  if(INTEL_CUSTOMIZATION)
    # CMPLRLLVM-40052: TODO: make the -no-opaque-pointers here based on
    # DPCPP_ENABLE_OPAQUE_POINTERS cmake variable
    list(APPEND opencl_flags_spirv
      -DINTEL_CUSTOMIZATION=1
      -disable-intel-proprietary-opts
      -no-opaque-pointers # SPIR-V is not quite ready with opaque pointers
    )
    list(PREPEND omp_cpp_flags --intel -DINTEL_CUSTOMIZATION=1
      -Xclang -no-opaque-pointers # SPIR-V is not quite ready with opaque ptrs
    )
  endif(INTEL_CUSTOMIZATION)

  set(opencl_device_rtl_required
    ${LIBOMPTARGET_LIBRARY_DIR}/libomptarget-opencl-required)
  set(opencl_device_rtl_required_obj
    ${LIBOMPTARGET_LIBRARY_DIR}/libomp-spirvdevicertl-required${objext})

  set(opencl_device_rtl_optional
    ${LIBOMPTARGET_LIBRARY_DIR}/libomptarget-opencl-optional)
  set(opencl_device_rtl_optional_obj
    ${LIBOMPTARGET_LIBRARY_DIR}/libomp-spirvdevicertl-optional${objext})

  # TODO: Set this to FALSE if it becomes redundant later.
  set(BUILD_ONE_DEVICE_RTL TRUE)

  if (BUILD_ONE_DEVICE_RTL)
    set(opencl_device_rtl ${LIBOMPTARGET_LIBRARY_DIR}/libomptarget-opencl)
    # We use libomp- prefix to unify the naming with libdevice parts.
    set(opencl_device_rtl_obj
      ${LIBOMPTARGET_LIBRARY_DIR}/libomp-spirvdevicertl${objext}
    )
  endif()

  # Set required tools
  set(ocl_cmplr $<TARGET_FILE:clang>)
  set(omp_cmplr $<TARGET_FILE:clang>)
  set(llvm_link $<TARGET_FILE:llvm-link>)
  set(llvm_spirv $<TARGET_FILE:llvm-spirv>)
  set(clang_offload_bundler $<TARGET_FILE:clang-offload-bundler>)

  macro(build_rtl rtl_name rtl_obj rtl_src)
    set(bc_all "")

    # Generate LLVM IR for each file
    foreach(src ${rtl_src})
      get_filename_component(src_abs ${src} ABSOLUTE)
      get_filename_component(src_ext ${src} EXT)
      get_filename_component(filename ${src} NAME_WE)
      if(${src_ext} MATCHES .cl)
        # Compile target code with ocl compiler and extract the LLVM IR
        add_custom_command(
          OUTPUT ${filename}.bc
          COMMAND ${ocl_cmplr} ${opencl_flags_spirv} -o ${filename}.tmp.bc
                  ${src_abs}
          # Regularize the IR early for printf.
          COMMAND ${llvm_spirv} -s -o ${filename}.bc ${filename}.tmp.bc
          DEPENDS ${src_abs} ${opencl_inc} ${ocl_cmplr} ${llvm_spirv}
          VERBATIM
        )
      else()
        # Compile target code with omp compiler and extract the LLVM IR
        add_custom_command(
          OUTPUT ${filename}.bc
          COMMAND ${omp_cmplr} ${omp_cpp_flags} -o ${filename}${objext}
                  ${src_abs}
          COMMAND ${clang_offload_bundler}
                  --unbundle --type=o --targets=openmp-spir64
                  --input=${filename}${objext} --output=${filename}.bc
          DEPENDS ${src_abs} ${omp_cmplr} ${clang_offload_bundler}
          VERBATIM
        )
      endif()
      list(APPEND bc_all ${filename}.bc)
    endforeach()

    set(rtl_tmp_bc ${rtl_name}.tmp.bc)
    set(rtl_reg_bc ${rtl_name}.reg.tmp.bc)
    set(rtl_bc ${rtl_name}.bc)

    # Generate LLVM IR for the device RTL.
    add_custom_command(
      OUTPUT ${rtl_tmp_bc}
      COMMAND ${llvm_link} -o ${rtl_tmp_bc} ${bc_all}
      DEPENDS ${bc_all} ${llvm_link}
      VERBATIM
    )

    # Generate regularized LLVM IR for the device RTL.
    add_custom_command(
      OUTPUT ${rtl_reg_bc}
      COMMAND ${llvm_spirv} -s -o ${rtl_reg_bc} ${rtl_tmp_bc}
      DEPENDS ${rtl_tmp_bc} ${llvm_spirv}
      VERBATIM
    )

    if(TRUE)
      # Annotate device RTL code with ITT API calls.
      # This should be done after the regularization, because the annotation
      # pass only recognizes __spirv calls, and does not recognize OpenCL calls.
      add_custom_command(
        OUTPUT ${rtl_bc}
        COMMAND opt -SPIRITTAnnotations -o ${rtl_bc} ${rtl_reg_bc}
        DEPENDS ${rtl_reg_bc} opt
        VERBATIM
      )
    else()
      # FIME: remove this soon.
      add_custom_command(
        OUTPUT ${rtl_bc}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${rtl_reg_bc} ${rtl_bc}
        DEPENDS ${rtl_reg_bc}
        VERBATIM
      )
    endif()
    list(APPEND opencl_device_rtl_bc_list ${rtl_bc})

    # Generate bundled object file for the device RTL with an "empty" host part.
    add_custom_command(
      OUTPUT ${rtl_obj}
      COMMAND ${CMAKE_COMMAND} -E echo > empty_host.c
      COMMAND ${ocl_cmplr} -c -o empty_host${objext} empty_host.c
      COMMAND ${clang_offload_bundler} --type=o
              --targets=${host_target_name},openmp-spir64,openmp-spir64_gen,openmp-spir64_x86_64
              --input=empty_host${objext} --input=${rtl_bc} --input=${rtl_bc}
              --input=${rtl_bc} --output=${rtl_obj}
      DEPENDS ${rtl_bc} ${ocl_cmplr} ${clang_offload_bundler}
      VERBATIM
    )
    list(APPEND opencl_device_rtl_obj_list ${rtl_obj})
  endmacro()

  set(opencl_device_rtl_bc_list "")
  set(opencl_device_rtl_obj_list "")

  build_rtl(${opencl_device_rtl_optional} ${opencl_device_rtl_optional_obj}
    "${opencl_src_optional}"
  )

  build_rtl(${opencl_device_rtl_required} ${opencl_device_rtl_required_obj}
    "${opencl_src_required}"
  )

  if(BUILD_ONE_DEVICE_RTL)
    build_rtl(${opencl_device_rtl} ${opencl_device_rtl_obj} "${opencl_src}")
  endif()

  add_custom_target(omptarget-opencl ALL
    DEPENDS ${opencl_device_rtl_bc_list} ${opencl_device_rtl_obj_list}
  )

  set(install_dest lib${LIBOMPTARGET_LIBDIR_SUFFIX})

  # FIXME: remove this from install, when IFX driver switches to
  #        the object file linking.
  install(FILES ${opencl_device_rtl_bc_list}
    COMPONENT omptarget-opencl-rtl
    DESTINATION ${install_dest}
  )
  install(FILES ${opencl_device_rtl_obj_list}
    COMPONENT omptarget-opencl-rtl
    DESTINATION ${install_dest}
  )
else()
  libomptarget_say("Not building OpenCL offloading device RTL: OpenCL SDK not found")
endif()
# end INTEL_COLLAB
