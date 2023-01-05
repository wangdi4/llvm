# INTEL_CUSTOMIZATION
#
# Copyright (C) 2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this
# software or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.

function(add_bc_file src cmplr clang_offload_bundler)
  cmake_parse_arguments(ARG "" "" "DEPENDS" ${ARGN})

  # Math functions must not be compiled with fast-math themselves
  if (WIN32)
    set(target_opts -Qopenmp-targets=spir64,spir64_gen=\"-fp-model=precise -fno-math-errno\" -Qopenmp-target-simd -Qopenmp-target-intel-proprietary-opts)
  else()
    set(target_opts -fopenmp-targets=spir64,spir64_gen=\"-fp-model=precise -fno-math-errno -mllvm -vpo-paropt-preserve-llvm-intrin\" -fopenmp-target-simd -fopenmp-target-intel-proprietary-opts)
  endif()

  set(clang_opts -I ${CMAKE_CURRENT_SOURCE_DIR}/imf -Xclang -fdeclare-spirv-builtins -Xclang -O2 -Xclang -fopenmp-target-declare-simd-spir -D__SPIR__)

  get_filename_component(name ${src} NAME_WLE)
  set(fname "libomp-device-svml-${name}")
  set(obj_file ${binary_dir}/${fname}${objext})
  set(bc_file_spir64 ${binary_dir}/${fname}.spir64.bc)
  set(bc_file_spir64_gen ${binary_dir}/${fname}.spir64_gen.bc)

  # Compile target code with omp compiler and extract the LLVM IR.
  add_custom_command(OUTPUT ${bc_file_spir64} ${bc_file_spir64_gen}
    COMMAND ${cmplr} ${omp_compile_opts} ${clang_opts}
            ${target_opts}
            ${src} -c -o ${obj_file}
    COMMAND ${clang_offload_bundler} --unbundle --type=o
            -targets=openmp-spir64,openmp-spir64_gen --input=${obj_file}
            --output=${bc_file_spir64} --output=${bc_file_spir64_gen}
    MAIN_DEPENDENCY ${src}
    DEPENDS ${ARG_DEPENDS}
            ${cmplr} ${clang_offload_bundler}
    )

  set(out_file_spir64 ${bc_file_spir64} PARENT_SCOPE)
  set(out_file_spir64_gen ${bc_file_spir64_gen} PARENT_SCOPE)
endfunction()

function(add_omp_imf_device_obj imf_lib)
  set(cmplr $<TARGET_FILE:clang>)
  set(clang_offload_bundler $<TARGET_FILE:clang-offload-bundler>)

  set(imf_src_dir ${CMAKE_CURRENT_SOURCE_DIR})

  set(OMP_LIBDEVICE 1)
  include(ImfSrcConcate)

  set(bc_files_spir64)
  set(bc_files_spir64_gen)
  foreach(src ${imf_fallback_src_list})
    add_bc_file(${imf_src_dir}/${src} ${cmplr} ${clang_offload_bundler})
    list(APPEND bc_files_spir64 "${out_file_spir64}")
    list(APPEND bc_files_spir64_gen "${out_file_spir64_gen}")
  endforeach()

  # link bc files.
  set(llvm_link $<TARGET_FILE:llvm-link>)
  set(bc_spir64 ${binary_dir}/libomp-device-svml.spir64.bc)
  set(bc_spir64_gen ${binary_dir}/libomp-device-svml.spir64_gen.bc)
  add_custom_command(OUTPUT ${bc_spir64} ${bc_spir64_gen}
    COMMAND ${llvm_link} -o ${bc_spir64} ${bc_files_spir64}
    COMMAND ${llvm_link} -o ${bc_spir64_gen} ${bc_files_spir64_gen}
    DEPENDS ${llvm_link} ${bc_files_spir64} ${bc_files_spir64_gen}
  )

  # Generate bundled object file for the device RTL with an "empty" host part.
  if (WIN32)
    set(host_target_name host-x86_64-pc-windows-msvc)
  else()
    set(host_target_name host-x86_64-unknown-linux-gnu)
  endif()

  add_custom_command(OUTPUT ${imf_lib}
    COMMAND ${CMAKE_COMMAND} -E echo > empty_host.c
    COMMAND ${cmplr} -c empty_host.c -o empty_host${objext}
    COMMAND ${clang_offload_bundler} --type=o
            --targets=${host_target_name},openmp-spir64,openmp-spir64_gen
            --input=empty_host${objext} --input=${bc_spir64}
            --input=${bc_spir64_gen} --output=${imf_lib}
    DEPENDS ${cmplr} ${clang_offload_bundler} ${bc_spir64} ${bc_spir64_gen}
  )
endfunction()

set(imf_lib ${binary_dir}/libomp-device-svml${objext})
add_omp_imf_device_obj(${imf_lib})
list(APPEND omplib_objs ${imf_lib})
# end INTEL_CUSTOMIZATION
