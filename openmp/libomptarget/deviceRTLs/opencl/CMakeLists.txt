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
  set(omp_src src/abi-atomic-fp.cpp)
  set(opencl_device_rtl_tmp_bc
    ${LIBOMPTARGET_LIBRARY_DIR}/libomptarget-opencl.tmp.bc
    )
  set(opencl_device_rtl_regularized_tmp_bc
    ${LIBOMPTARGET_LIBRARY_DIR}/libomptarget-opencl.reg.tmp.bc
    )
  set(opencl_device_rtl_bc ${LIBOMPTARGET_LIBRARY_DIR}/libomptarget-opencl.bc)
  if (WIN32)
    set(objext .obj)
    set(host_target_name host-x86_64-pc-windows-msvc)
  else()
    set(objext .o)
    set(host_target_name host-x86_64-unknown-linux-gnu)
  endif()
  # We use libomp- prefix to unify the naming with libdevice parts.
  set(opencl_device_rtl_obj
    ${LIBOMPTARGET_LIBRARY_DIR}/libomp-spirvdevicertl${objext}
    )
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
    list(APPEND opencl_flags_spirv
      -DINTEL_CUSTOMIZATION=1
      -disable-intel-proprietary-opts
      )
    list(PREPEND omp_cpp_flags --intel -DINTEL_CUSTOMIZATION=1)
  endif(INTEL_CUSTOMIZATION)

  add_custom_target(omptarget-opencl ALL
    DEPENDS ${opencl_device_rtl_bc} ${opencl_device_rtl_obj}
    )

  # Set required tools
  set(ocl_cmplr $<TARGET_FILE:clang>)
  set(omp_cmplr $<TARGET_FILE:clang>)
  set(llvm_link $<TARGET_FILE:llvm-link>)
  set(llvm_spirv $<TARGET_FILE:llvm-spirv>)
  set(clang_offload_bundler $<TARGET_FILE:clang-offload-bundler>)

  # Generate LLVM IR and SPIR-V from the OpenCL code
  set(bc_all "")

  # Generate LLVM IR for each file
  foreach(src ${opencl_src})
    get_filename_component(cl_src ${src} ABSOLUTE)
    get_filename_component(filename ${src} NAME_WE)
    add_custom_command(
      OUTPUT ${filename}.bc
      COMMAND ${ocl_cmplr} ${opencl_flags_spirv} -o ${filename}.tmp.bc ${cl_src}
      # Regularize the IR early for printf.
      COMMAND ${llvm_spirv} -s -o ${filename}.bc ${filename}.tmp.bc
      DEPENDS ${cl_src} ${opencl_inc} ${ocl_cmplr} ${llvm_spirv}
      VERBATIM
      )
    list(APPEND bc_all ${filename}.bc)
  endforeach()

  # Compile target code with omp compiler and extract the LLVM IR
  foreach(src ${omp_src})
    get_filename_component(omp_src ${src} ABSOLUTE)
    get_filename_component(filename ${src} NAME_WE)
    add_custom_command(
      OUTPUT ${filename}.bc
      COMMAND ${omp_cmplr} ${omp_cpp_flags} -o ${filename}${objext} ${omp_src}
      COMMAND ${clang_offload_bundler}
              --unbundle --type=o --targets=openmp-spir64
              --inputs=${filename}${objext} --outputs=${filename}.bc
      DEPENDS ${omp_src} ${omp_cmplr} ${clang_offload_bundler}
      VERBATIM
      )
    list(APPEND bc_all ${filename}.bc)
  endforeach()

  # Generate LLVM IR for the device RTL.
  add_custom_command(
    OUTPUT ${opencl_device_rtl_tmp_bc}
    COMMAND ${llvm_link} -o ${opencl_device_rtl_tmp_bc} ${bc_all}
    DEPENDS ${bc_all} ${llvm_link}
    VERBATIM
    )

  # Generate regularized LLVM IR for the device RTL.
  add_custom_command(
    OUTPUT ${opencl_device_rtl_regularized_tmp_bc}
    COMMAND ${llvm_spirv} -s -o ${opencl_device_rtl_regularized_tmp_bc}
            ${opencl_device_rtl_tmp_bc}
    DEPENDS ${opencl_device_rtl_tmp_bc} ${llvm_spirv}
    VERBATIM
    )

  if(TRUE)
    # Annotate device RTL code with ITT API calls.
    # This should be done after the regularization, because the annotation
    # pass only recognizes __spirv calls, and does not recognize OpenCL calls.
    add_custom_command(
      OUTPUT ${opencl_device_rtl_bc}
      COMMAND opt -SPIRITTAnnotations -o ${opencl_device_rtl_bc}
              ${opencl_device_rtl_regularized_tmp_bc}
      DEPENDS ${opencl_device_rtl_regularized_tmp_bc} opt
      VERBATIM
      )
  else()
    # FIME: remove this soon.
    add_custom_command(
      OUTPUT ${opencl_device_rtl_bc}
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              ${opencl_device_rtl_regularized_tmp_bc} ${opencl_device_rtl_bc}
      DEPENDS ${opencl_device_rtl_regularized_tmp_bc}
      VERBATIM
      )
  endif()

  # Generate bundled object file for the device RTL with an "empty" host part.
  add_custom_command(
    OUTPUT ${opencl_device_rtl_obj}
    COMMAND ${CMAKE_COMMAND} -E echo > empty_host.c
    COMMAND clang -c -o empty_host${objext} empty_host.c
    COMMAND clang-offload-bundler --type=o
            --targets=${host_target_name},openmp-spir64,openmp-spir64_gen,openmp-spir64_x86_64
            --inputs=empty_host${objext},${opencl_device_rtl_bc},${opencl_device_rtl_bc},${opencl_device_rtl_bc}
            --outputs=${opencl_device_rtl_obj}
    DEPENDS ${opencl_device_rtl_bc} ${ocl_cmplr} ${clang_offload_bundler}
    VERBATIM
    )

  set(install_dest lib${LIBOMPTARGET_LIBDIR_SUFFIX})

  # FIXME: remove this from install, when IFX driver switches to
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
