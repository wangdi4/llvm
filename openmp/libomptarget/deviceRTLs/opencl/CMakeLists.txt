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
  set(opencl_src
    src/abi.cl
    src/abi-atomic.cl
    src/abi-loop.cl
    src/abi-parallel.cl
    src/api.cl
    src/omptarget-opencl.cl
    )
  set(opencl_device_rtl_tmp_bc
    ${LIBOMPTARGET_LIBRARY_DIR}/libomptarget-opencl.tmp.bc
    )
  set(opencl_device_rtl_bc ${LIBOMPTARGET_LIBRARY_DIR}/libomptarget-opencl.bc)
  if (WIN32)
    set(objext .obj)
    set(host_target_name host-x86_64-pc-windows-msvc)
  else()
    set(objext .o)
    set(host_target_name host-x86_64-unknown-linux-gnu)
  endif()
  set(opencl_device_rtl_obj
    ${LIBOMPTARGET_LIBRARY_DIR}/libomptarget-spirvdevicertl${objext}
    )
  set(opencl_flags_spirv
    -cc1 -emit-llvm-bc -triple spir64-unknown-unknown -cl-std=CL2.0
    -disable-llvm-passes -include opencl-c.h -DINTEL_COLLAB=1
    )
  if("$ENV{LIBOMPTARGET_NO_DOUBLE_PRECISION}" STREQUAL "1")
    list(APPEND opencl_flags_spirv -DHAVE_FP64_SUPPORT=0)
  else()
    list(APPEND opencl_flags_spirv -DHAVE_FP64_SUPPORT=1)
  endif()
  if(INTEL_CUSTOMIZATION)
    list(APPEND opencl_flags_spirv
      -DINTEL_CUSTOMIZATION=1
      -disable-intel-proprietary-opts
      )
  endif(INTEL_CUSTOMIZATION)

  add_custom_target(omptarget-opencl ALL
    DEPENDS ${opencl_device_rtl_bc} ${opencl_device_rtl_obj}
    )

  # Generate LLVM IR and SPIR-V from the OpenCL code
  set(bc_all "")
  foreach(src ${opencl_src})
    get_filename_component(cl_src ${src} ABSOLUTE)
    get_filename_component(filename ${src} NAME_WE)
    add_custom_command(
      OUTPUT ${filename}.bc
      COMMAND clang ${opencl_flags_spirv} -o ${filename}.bc ${cl_src}
      DEPENDS ${cl_src} ${opencl_inc} clang
      VERBATIM
      )
    list(APPEND bc_all ${filename}.bc)
  endforeach()

  # Generate LLVM IR for the device RTL.
  add_custom_command(
    OUTPUT ${opencl_device_rtl_tmp_bc}
    COMMAND llvm-link -o ${opencl_device_rtl_tmp_bc} ${bc_all}
    DEPENDS ${bc_all} llvm-link
    VERBATIM
    )

  # Generate regularized LLVM IR for the device RTL.
  add_custom_command(
    OUTPUT ${opencl_device_rtl_bc}
    COMMAND llvm-spirv -s -o ${opencl_device_rtl_bc} ${opencl_device_rtl_tmp_bc}
    DEPENDS ${opencl_device_rtl_tmp_bc} llvm-spirv
    VERBATIM
    )

  # Generate bundled object file for the device RTL with an "empty" host part.
  add_custom_command(
    OUTPUT ${opencl_device_rtl_obj}
    COMMAND ${CMAKE_COMMAND} -E echo > empty_host.c
    COMMAND clang -c -o empty_host${objext} empty_host.c
    COMMAND clang-offload-bundler --type=o
            --targets=${host_target_name},openmp-spir64
            --inputs=empty_host${objext},${opencl_device_rtl_bc}
            --outputs=${opencl_device_rtl_obj}
    DEPENDS ${opencl_device_rtl_bc} clang clang-offload-bundler
    VERBATIM
    )

  set(install_dest lib${LIBOMPTARGET_LIBDIR_SUFFIX})

  # FIXME: remove this from install, when the driver switches to
  #        the object file linking.
  install(FILES ${opencl_device_rtl_bc}
    COMPONENT omptarget-opencl-rtl
    DESTINATION ${install_dest}
    )
  install(FILES ${opencl_device_rtl_obj}
    COMPONENT omptarget-opencl-rtl
    DESTINATION ${install_dest}
    )
else()
  libomptarget_say("Not building OpenCL offloading device RTL: OpenCL SDK not found")
endif()
# end INTEL_COLLAB
