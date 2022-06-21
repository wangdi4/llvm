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
set(obj_binary_dir "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
if (WIN32)
  set(lib-suffix obj)
  set(spv_binary_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  set(install_dest_spv bin)
  set(devicelib_host_static sycl-devicelib-host.lib)
else()
  set(lib-suffix o)
  set(spv_binary_dir "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
  set(install_dest_spv lib${LLVM_LIBDIR_SUFFIX})
  set(devicelib_host_static libsycl-devicelib-host.a)
endif()
set(install_dest_lib lib${LLVM_LIBDIR_SUFFIX})

set(clang $<TARGET_FILE:clang>)
<<<<<<< HEAD
set(llvm-link $<TARGET_FILE:llvm-link>)
set(llc $<TARGET_FILE:llc>)
set(llvm-spirv $<TARGET_FILE:llvm-spirv>)
set(llvm-ar $<TARGET_FILE:llvm-ar>)
set(clang-offload-bundler $<TARGET_FILE:clang-offload-bundler>)
=======
set(llvm-ar $<TARGET_FILE:llvm-ar>)
>>>>>>> d094266ae1653e705c4c0e31e8e5a42fa0a5f095

string(CONCAT sycl_targets_opt
  "-fsycl-targets="
  "spir64_x86_64-unknown-unknown,"
  "spir64_gen-unknown-unknown,"
  "spir64_fpga-unknown-unknown,"
  "spir64-unknown-unknown")

set(compile_opts
# INTEL_COLLAB
  -DINTEL_COLLAB
  -DINTEL_CUSTOMIZATION
# end INTEL_COLLAB
  # suppress an error about SYCL_EXTERNAL being used for
  # a function with a raw pointer parameter.
  -Wno-sycl-strict
  # Disable warnings for the host compilation, where
  # we declare all functions as 'static'.
  -Wno-undefined-internal
  -sycl-std=2020
  )

if (WIN32)
  list(APPEND compile_opts -D_ALLOW_RUNTIME_LIBRARY_MISMATCH)
  list(APPEND compile_opts -D_ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH)
endif()

add_custom_target(libsycldevice-obj)
add_custom_target(libsycldevice-spv)

add_custom_target(libsycldevice DEPENDS
  libsycldevice-obj
  libsycldevice-spv)

function(add_devicelib_obj obj_filename)
  cmake_parse_arguments(OBJ  "" "" "SRC;DEP;EXTRA_ARGS" ${ARGN})
  set(devicelib-obj-file ${obj_binary_dir}/${obj_filename}.${lib-suffix})
  add_custom_command(OUTPUT ${devicelib-obj-file}
                     COMMAND ${clang} -fsycl -c
                             ${compile_opts} ${sycl_targets_opt} ${OBJ_EXTRA_ARGS}
                             ${CMAKE_CURRENT_SOURCE_DIR}/${OBJ_SRC}
                             -o ${devicelib-obj-file}
                     MAIN_DEPENDENCY ${OBJ_SRC}
                     DEPENDS ${OBJ_DEP}
                     VERBATIM)
  set(devicelib-obj-target ${obj_filename}-obj)
  add_custom_target(${devicelib-obj-target} DEPENDS ${devicelib-obj-file})
  add_dependencies(libsycldevice-obj ${devicelib-obj-target})
  install(FILES ${devicelib-obj-file}
          DESTINATION ${install_dest_lib}
          COMPONENT libsycldevice)
endfunction()

function(add_devicelib_spv spv_filename)
  cmake_parse_arguments(SPV  "" "" "SRC;DEP;EXTRA_ARGS" ${ARGN})
  set(devicelib-spv-file ${spv_binary_dir}/${spv_filename}.spv)
  add_custom_command(OUTPUT ${devicelib-spv-file}
                     COMMAND ${clang} -fsycl-device-only -fno-sycl-use-bitcode
                             ${compile_opts} ${SPV_EXTRA_ARGS}
                             ${CMAKE_CURRENT_SOURCE_DIR}/${SPV_SRC}
                             -o ${devicelib-spv-file}
                     MAIN_DEPENDENCY ${SPV_SRC}
                     DEPENDS ${SPV_DEP}
                     VERBATIM)
  set(devicelib-spv-target ${spv_filename}-spv)
  add_custom_target(${devicelib-spv-target} DEPENDS ${devicelib-spv-file})
  add_dependencies(libsycldevice-spv ${devicelib-spv-target})
  install(FILES ${devicelib-spv-file}
          DESTINATION ${install_dest_spv}
          COMPONENT libsycldevice)
endfunction()

function(add_fallback_devicelib fallback_filename)
  cmake_parse_arguments(FB "" "" "SRC;DEP;EXTRA_ARGS" ${ARGN})
  add_devicelib_spv(${fallback_filename} SRC ${FB_SRC} DEP ${FB_DEP} EXTRA_ARGS ${FB_EXTRA_ARGS})
  add_devicelib_obj(${fallback_filename} SRC ${FB_SRC} DEP ${FB_DEP} EXTRA_ARGS ${FB_EXTRA_ARGS})
endfunction()

set(crt_obj_deps wrapper.h device.h spirv_vars.h sycl-compiler)
set(complex_obj_deps device_complex.h device.h sycl-compiler)
set(cmath_obj_deps device_math.h device.h sycl-compiler)
set(imf_obj_deps device_imf.hpp imf_half.hpp device.h sycl-compiler)
set(itt_obj_deps device_itt.h spirv_vars.h device.h sycl-compiler)

add_devicelib_obj(libsycl-itt-stubs SRC itt_stubs.cpp DEP ${itt_obj_deps})
add_devicelib_obj(libsycl-itt-compiler-wrappers SRC itt_compiler_wrappers.cpp DEP ${itt_obj_deps})
add_devicelib_obj(libsycl-itt-user-wrappers SRC itt_user_wrappers.cpp DEP ${itt_obj_deps})

add_devicelib_obj(libsycl-crt SRC crt_wrapper.cpp DEP ${crt_obj_deps})
add_devicelib_obj(libsycl-complex SRC complex_wrapper.cpp DEP ${complex_obj_deps})
add_devicelib_obj(libsycl-complex-fp64 SRC complex_wrapper_fp64.cpp DEP ${complex_obj_deps} )
add_devicelib_obj(libsycl-cmath SRC cmath_wrapper.cpp DEP ${cmath_obj_deps})
add_devicelib_obj(libsycl-cmath-fp64 SRC cmath_wrapper_fp64.cpp DEP ${cmath_obj_deps} )
add_devicelib_obj(libsycl-imf SRC imf_wrapper.cpp DEP ${imf_obj_deps})
add_devicelib_obj(libsycl-imf-fp64 SRC imf_wrapper_fp64.cpp DEP ${imf_obj_deps})
if(WIN32)
add_devicelib_obj(libsycl-msvc-math SRC msvc_math.cpp DEP ${cmath_obj_deps})
endif()

add_fallback_devicelib(libsycl-fallback-cassert SRC fallback-cassert.cpp DEP ${crt_obj_deps} EXTRA_ARGS -fno-sycl-instrument-device-code)
add_fallback_devicelib(libsycl-fallback-cstring SRC fallback-cstring.cpp DEP ${crt_obj_deps})
add_fallback_devicelib(libsycl-fallback-complex SRC fallback-complex.cpp DEP ${complex_obj_deps})
add_fallback_devicelib(libsycl-fallback-complex-fp64 SRC fallback-complex-fp64.cpp DEP ${complex_obj_deps} )
add_fallback_devicelib(libsycl-fallback-cmath SRC fallback-cmath.cpp DEP ${cmath_obj_deps})
add_fallback_devicelib(libsycl-fallback-cmath-fp64 SRC fallback-cmath-fp64.cpp DEP ${cmath_obj_deps})

<<<<<<< HEAD
# imf fallback is different, we have many separate sources instead of single one including all functions.
# So, we need to combine all LLVM IR to a complete one and run llvm-spirv for it.
file(MAKE_DIRECTORY ${obj_binary_dir}/libdevice)
set(bc_binary_dir ${obj_binary_dir}/libdevice)

set(fallback-imf-src imf_utils/float_convert.cpp
                     imf_utils/half_convert.cpp
                     imf_utils/integer_misc.cpp
                     imf/imf_inline_fp32.cpp)
set(fallback-imf-fp64-src imf_utils/double_convert.cpp
                          imf/imf_inline_fp64.cpp)
set(wrapper-imf-src imf_wrapper.cpp imf_wrapper_fp64.cpp)
set(imf-src ${wrapper-imf-src} ${fallback-imf-src} ${fallback-imf-fp64-src})

add_custom_target(imf-fallback-spv
                  COMMAND ${llvm-spirv}
                           ${bc_binary_dir}/fallback-imf-spir64-unknown-unknown.bc
                           -o ${spv_binary_dir}/libsycl-fallback-imf.spv)
add_custom_target(imf-fp64-fallback-spv
                  COMMAND ${llvm-spirv}
                  ${bc_binary_dir}/fallback-imf-fp64-spir64-unknown-unknown.bc
                  -o ${spv_binary_dir}/libsycl-fallback-imf-fp64.spv)
     
add_dependencies(libsycldevice-spv imf-fallback-spv)
add_dependencies(libsycldevice-spv imf-fp64-fallback-spv)
=======
file(MAKE_DIRECTORY ${obj_binary_dir}/libdevice)
set(imf_fallback_src_dir ${obj_binary_dir}/libdevice)
set(imf_src_dir ${CMAKE_CURRENT_SOURCE_DIR})
set(imf_fallback_fp32_deps device.h device_imf.hpp imf_half.hpp
                           imf_utils/integer_misc.cpp
                           imf_utils/float_convert.cpp
                           imf_utils/half_convert.cpp
                           imf/imf_inline_fp32.cpp)
set(imf_fallback_fp64_deps device.h device_imf.hpp imf_half.hpp
                           imf_utils/double_convert.cpp
                           imf/imf_inline_fp64.cpp)
set(imf_fp32_fallback_src ${imf_fallback_src_dir}/imf_fp32_fallback.cpp)
set(imf_fp64_fallback_src ${imf_fallback_src_dir}/imf_fp64_fallback.cpp)

add_custom_command(OUTPUT ${imf_fp32_fallback_src}
                   COMMAND ${CMAKE_COMMAND} -D SRC_DIR=${imf_src_dir}
                                            -D DEST_DIR=${imf_fallback_src_dir}
                                            -D FP64=0
                                            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/ImfSrcConcate.cmake
                   DEPENDS ${imf_fallback_fp32_deps})

add_custom_command(OUTPUT ${imf_fp64_fallback_src}
                   COMMAND ${CMAKE_COMMAND} -D SRC_DIR=${imf_src_dir}
                                            -D DEST_DIR=${imf_fallback_src_dir}
                                            -D FP64=1
                                            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/ImfSrcConcate.cmake
                   DEPENDS ${imf_fallback_fp64_deps})

add_custom_target(get_imf_fallback_fp32  DEPENDS ${imf_fp32_fallback_src})
add_custom_command(OUTPUT ${spv_binary_dir}/libsycl-fallback-imf.spv
                   COMMAND ${clang} -fsycl-device-only -fno-sycl-use-bitcode
                           ${compile_opts} -I ${CMAKE_CURRENT_SOURCE_DIR}/imf
                           ${imf_fp32_fallback_src}
                           -o ${spv_binary_dir}/libsycl-fallback-imf.spv
                   DEPENDS ${imf_fallback_fp32_deps} get_imf_fallback_fp32 sycl-compiler
                   VERBATIM)

add_custom_command(OUTPUT ${obj_binary_dir}/libsycl-fallback-imf.${lib-suffix}
                   COMMAND ${clang} -fsycl -c
                           ${compile_opts} ${sycl_targets_opt}
                           ${imf_fp32_fallback_src} -I ${CMAKE_CURRENT_SOURCE_DIR}/imf
                           -o ${obj_binary_dir}/libsycl-fallback-imf.${lib-suffix}
                   DEPENDS ${imf_fallback_fp32_deps} get_imf_fallback_fp32 sycl-compiler
                   VERBATIM)

add_custom_command(OUTPUT ${obj_binary_dir}/fallback-imf-fp32-host.${lib-suffix}
                   COMMAND ${clang} -c -D__LIBDEVICE_HOST_IMPL__
                           -I ${CMAKE_CURRENT_SOURCE_DIR}/imf
                           ${imf_fp32_fallback_src}
                           -o ${obj_binary_dir}/fallback-imf-fp32-host.${lib-suffix}
                   DEPENDS ${imf_fallback_fp32_deps} get_imf_fallback_fp32 sycl-compiler
                   VERBATIM)

add_custom_target(get_imf_fallback_fp64  DEPENDS ${imf_fp64_fallback_src})
add_custom_command(OUTPUT ${spv_binary_dir}/libsycl-fallback-imf-fp64.spv
                   COMMAND ${clang} -fsycl-device-only -fno-sycl-use-bitcode
                           ${compile_opts} -I ${CMAKE_CURRENT_SOURCE_DIR}/imf
                           ${imf_fp64_fallback_src}
                           -o ${spv_binary_dir}/libsycl-fallback-imf-fp64.spv
                   DEPENDS ${imf_fallback_fp64_deps} get_imf_fallback_fp64 sycl-compiler
                   VERBATIM)

add_custom_command(OUTPUT ${obj_binary_dir}/libsycl-fallback-imf-fp64.${lib-suffix}
                   COMMAND ${clang} -fsycl -c -I ${CMAKE_CURRENT_SOURCE_DIR}/imf
                           ${compile_opts} ${sycl_targets_opt}
                           ${imf_fp64_fallback_src}
                           -o ${obj_binary_dir}/libsycl-fallback-imf-fp64.${lib-suffix}
                   DEPENDS ${imf_fallback_fp64_deps} get_imf_fallback_fp64 sycl-compiler
                   VERBATIM)

add_custom_command(OUTPUT ${obj_binary_dir}/fallback-imf-fp64-host.${lib-suffix}
                   COMMAND ${clang} -c -D__LIBDEVICE_HOST_IMPL__
                           -I ${CMAKE_CURRENT_SOURCE_DIR}/imf
                           ${imf_fp64_fallback_src}
                           -o ${obj_binary_dir}/fallback-imf-fp64-host.${lib-suffix}
                   DEPENDS ${imf_fallback_fp64_deps} get_imf_fallback_fp64 sycl-compiler
                   VERBATIM)

add_custom_target(imf_fallback_fp32_spv DEPENDS ${spv_binary_dir}/libsycl-fallback-imf.spv)
add_custom_target(imf_fallback_fp32_obj DEPENDS ${obj_binary_dir}/libsycl-fallback-imf.${lib-suffix})
add_custom_target(imf_fallback_fp32_host_obj DEPENDS ${obj_binary_dir}/fallback-imf-fp32-host.${lib-suffix})
add_dependencies(libsycldevice-spv imf_fallback_fp32_spv)
add_dependencies(libsycldevice-obj imf_fallback_fp32_obj)

add_custom_target(imf_fallback_fp64_spv DEPENDS ${spv_binary_dir}/libsycl-fallback-imf-fp64.spv)
add_custom_target(imf_fallback_fp64_obj DEPENDS ${obj_binary_dir}/libsycl-fallback-imf-fp64.${lib-suffix})
add_custom_target(imf_fallback_fp64_host_obj DEPENDS ${obj_binary_dir}/fallback-imf-fp64-host.${lib-suffix})
add_dependencies(libsycldevice-spv imf_fallback_fp64_spv)
add_dependencies(libsycldevice-obj imf_fallback_fp64_obj)

add_custom_command(OUTPUT ${obj_binary_dir}/imf-fp32-host.${lib-suffix}
                   COMMAND ${clang} -c -D__LIBDEVICE_HOST_IMPL__
                           ${CMAKE_CURRENT_SOURCE_DIR}/imf_wrapper.cpp
                           -o ${obj_binary_dir}/imf-fp32-host.${lib-suffix}
                   MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/imf_wrapper.cpp
                   DEPENDS ${imf_obj_deps}
                   VERBATIM)

add_custom_command(OUTPUT ${obj_binary_dir}/imf-fp64-host.${lib-suffix}
                   COMMAND ${clang} -c -D__LIBDEVICE_HOST_IMPL__
                           ${CMAKE_CURRENT_SOURCE_DIR}/imf_wrapper_fp64.cpp
                           -o ${obj_binary_dir}/imf-fp64-host.${lib-suffix}
                   MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/imf_wrapper_fp64.cpp
                   DEPENDS ${imf_obj_deps}
                   VERBATIM)

add_custom_target(imf_fp32_host_obj DEPENDS ${obj_binary_dir}/imf-fp32-host.${lib-suffix})
add_custom_target(imf_fp64_host_obj DEPENDS ${obj_binary_dir}/imf-fp64-host.${lib-suffix})
add_custom_target(imf_host_obj
                  COMMAND ${llvm-ar} rcs ${obj_binary_dir}/${devicelib_host_static}
                          ${obj_binary_dir}/imf-fp32-host.${lib-suffix}
                          ${obj_binary_dir}/fallback-imf-fp32-host.${lib-suffix}
                          ${obj_binary_dir}/imf-fp64-host.${lib-suffix}
                          ${obj_binary_dir}/fallback-imf-fp64-host.${lib-suffix}
                  DEPENDS imf_fp32_host_obj imf_fallback_fp32_host_obj imf_fp64_host_obj imf_fallback_fp64_host_obj sycl-compiler
                  VERBATIM)
add_dependencies(libsycldevice-obj imf_host_obj)
>>>>>>> d094266ae1653e705c4c0e31e8e5a42fa0a5f095
install(FILES ${spv_binary_dir}/libsycl-fallback-imf.spv
              ${spv_binary_dir}/libsycl-fallback-imf-fp64.spv
        DESTINATION ${install_dest_spv}
        COMPONENT libsycldevice)

<<<<<<< HEAD
set(sycl_offload_targets sycl-spir64_x86_64-unknown-unknown
    sycl-spir64_gen-unknown-unknown
    sycl-spir64_fpga-unknown-unknown
    sycl-spir64-unknown-unknow
    host-x86_64-unknown-linux-gnu)

string(REPLACE ";" "," sycl_offload_targets "${sycl_offload_targets}")
set(imf-offload-inputs ${bc_binary_dir}/fallback-imf-spir64-unknown-unknown.bc
                       ${bc_binary_dir}/fallback-imf-spir64_x86_64-unknown-unknown.bc
                       ${bc_binary_dir}/fallback-imf-spir64_gen-unknown-unknown.bc
                       ${bc_binary_dir}/fallback-imf-spir64_fpga-unknown-unknown.bc
                       ${bc_binary_dir}/fallback-imf-dummy-host.bc)
string(REPLACE ";" "," imf-offload-inputs "${imf-offload-inputs}")
add_custom_target(imf-fallback-obj
                 COMMAND ${clang-offload-bundler} -type=o -targets=${sycl_offload_targets}
                 -outputs=${obj_binary_dir}/libsycl-fallback-imf.${lib-suffix}
                 -inputs=${imf-offload-inputs})

add_dependencies(libsycldevice-obj imf-fallback-obj)

set(imf-fp64-offload-inputs ${bc_binary_dir}/fallback-imf-fp64-spir64-unknown-unknown.bc
                            ${bc_binary_dir}/fallback-imf-fp64-spir64_x86_64-unknown-unknown.bc
                            ${bc_binary_dir}/fallback-imf-fp64-spir64_gen-unknown-unknown.bc
                            ${bc_binary_dir}/fallback-imf-fp64-spir64_fpga-unknown-unknown.bc
                            ${bc_binary_dir}/fallback-imf-fp64-dummy-host.bc)
string(REPLACE ";" "," imf-fp64-offload-inputs "${imf-fp64-offload-inputs}")
add_custom_target(imf-fp64-fallback-obj
                 COMMAND ${clang-offload-bundler} -type=o -targets=${sycl_offload_targets}
                 -outputs=${obj_binary_dir}/libsycl-fallback-imf-fp64.${lib-suffix}
                 -inputs=${imf-fp64-offload-inputs})

add_dependencies(libsycldevice-obj imf-fp64-fallback-obj)

install(FILES ${obj_binary_dir}/libsycl-fallback-imf.${lib-suffix}
              ${obj_binary_dir}/libsycl-fallback-imf-fp64.${lib-suffix}
        DESTINATION ${install_dest_lib}
        COMPONENT libsycldevice)

function(add_devicelib_bc src_file sycl_target)
  cmake_parse_arguments(BC "" "" "DEPS;DEPED" ${ARGN})
  get_filename_component(fn ${src_file} NAME_WE)
  set(temp_bc_fn ${fn}-${sycl_target}.bc)
  set(devicelib-bc ${bc_binary_dir}/${temp_bc_fn})
  if(sycl_target STREQUAL "dummy-host")
    set(bc_compile_flags -c -emit-llvm)
  elseif(sycl_target STREQUAL "host")
    set(bc_compile_flags -c -emit-llvm -D__LIBDEVICE_HOST_IMPL__)
  else()
    set(bc_compile_flags -fsycl -fsycl-device-only -fsycl-targets=${sycl_target})
  endif()
  if (WIN32)
    list(APPEND bc_compile_flags -D_ALLOW_RUNTIME_LIBRARY_MISMATCH)
    list(APPEND bc_compile_flags -D_ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH)
  endif()
  add_custom_command(OUTPUT ${devicelib-bc}
                     COMMAND ${clang} ${bc_compile_flags}
                             ${CMAKE_CURRENT_SOURCE_DIR}/${src_file}
                             -o ${devicelib-bc}
                     MAIN_DEPENDENCY ${src_file}
                     DEPENDS ${BC_DEPS}
                     VERBATIM)
  add_custom_target(${temp_bc_fn} DEPENDS ${devicelib-bc})
  add_dependencies(${BC_DEPED} ${temp_bc_fn})
endfunction()

function(merge_devicelib_bc bc_filename sycl_target)
  cmake_parse_arguments(FBC  "" "" "SRCS;DEPS;DEPED" ${ARGN})
  set(bc_file_list)
  foreach(src ${FBC_SRCS})
    get_filename_component(fn ${src} NAME_WE)
    set(temp_bc_fn ${fn}-${sycl_target}.bc)
    list(APPEND bc_file_list ${bc_binary_dir}/${temp_bc_fn})
  endforeach()
  set(bc_target ${bc_filename}-${sycl_target})
  add_custom_target(${bc_target}
                    COMMAND ${llvm-link} ${bc_file_list} -o ${bc_binary_dir}/${bc_target}.bc
                    VERBATIM)
  foreach(src ${FBC_SRCS})
    add_devicelib_bc(${src} ${sycl_target}
                  DEPS ${FBC_DEPS}
                  DEPED ${bc_target})
  endforeach()
  foreach(deped ${FBC_DEPED})
    add_dependencies(${deped} ${bc_target})
  endforeach()
endfunction()

set(imf_sycl_targets spir64_x86_64-unknown-unknown
    spir64_gen-unknown-unknown
    spir64_fpga-unknown-unknown
    spir64-unknown-unknown
    dummy-host)

foreach(imf_target ${imf_sycl_targets})
  if(imf_target STREQUAL "spir64-unknown-unknown")
    set(deped_list imf-fallback-obj imf-fallback-spv)
    set(deped64_list imf-fp64-fallback-obj imf-fp64-fallback-spv)
  else()
    set(deped_list imf-fallback-obj)
    set(deped64_list imf-fp64-fallback-obj)
  endif()
  merge_devicelib_bc(fallback-imf ${imf_target}
                     SRCS ${fallback-imf-src}
                     DEPS ${imf_obj_deps}
                     DEPED ${deped_list})

  merge_devicelib_bc(fallback-imf-fp64 ${imf_target}
                     SRCS ${fallback-imf-fp64-src}
                     DEPS ${imf_obj_deps}
                     DEPED ${deped64_list})
endforeach()

add_custom_target(imf-host-obj
                  COMMAND ${llc} -filetype=obj
                          ${bc_binary_dir}/imf-host.bc -o
                          ${bc_binary_dir}/imf-host.${lib-suffix}
                  COMMAND ${llvm-ar} rcs ${obj_binary_dir}/${devicelib_host_static} ${bc_binary_dir}/imf-host.${lib-suffix}
                  VERBATIM)

add_dependencies(libsycldevice imf-host-obj)

install(FILES ${obj_binary_dir}/${devicelib_host_static}
        DESTINATION ${install_dest_lib}
        COMPONENT libsycldevice)

merge_devicelib_bc(imf host
                   SRCS ${imf-src}
                   DEPS ${imf_obj_deps}
                   DEPED imf-host-obj)
=======
install(FILES ${obj_binary_dir}/libsycl-fallback-imf.${lib-suffix}
              ${obj_binary_dir}/libsycl-fallback-imf-fp64.${lib-suffix}
              ${obj_binary_dir}/${devicelib_host_static}
        DESTINATION ${install_dest_lib}
        COMPONENT libsycldevice)
>>>>>>> d094266ae1653e705c4c0e31e8e5a42fa0a5f095
