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
set(llvm-ar $<TARGET_FILE:llvm-ar>)

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

file(MAKE_DIRECTORY ${obj_binary_dir}/libdevice)
set(imf_fallback_src_dir ${obj_binary_dir}/libdevice)
set(imf_src_dir ${CMAKE_CURRENT_SOURCE_DIR})
set(imf_fallback_fp32_deps device.h device_imf.hpp imf_half.hpp
                           imf_utils/integer_misc.cpp
                           imf_utils/float_convert.cpp
                           imf_utils/half_convert.cpp
                           imf_utils/simd_emulate.cpp
                           # INTEL_CUSTOMIZATION
                           imf/_imf_include_fp32.hpp
                           imf/intel/erfinv_s_ha.cpp
                           imf/intel/cos_s_ha.cpp
                           imf/intel/cbrt_s_la.cpp
                           imf/intel/atan_s_ha.cpp
                           imf/intel/atanh_s_la.cpp
                           imf/intel/atan2_s_la.cpp
                           imf/intel/asin_s_la.cpp
                           imf/intel/asinh_s_la.cpp
                           imf/intel/acos_s_ha.cpp
                           imf/intel/acosh_s_la.cpp
                           imf/intel/erf_s_ha.cpp
                           imf/intel/cospi_s_ha.cpp
                           imf/intel/cosh_s_la.cpp
                           imf/intel/frexp_s_xa.cpp
                           imf/intel/fmod_s_xa.cpp
                           imf/intel/fdim_s_ha.cpp
                           imf/intel/exp_s_la.cpp
                           imf/intel/expm1_s_ha.cpp
                           imf/intel/exp2_s_la.cpp
                           imf/intel/exp10_s_la.cpp
                           imf/intel/erfc_s_la.cpp
                           imf/intel/erfcinv_s_la.cpp
                           imf/intel/norm3d_s_la.cpp
                           imf/intel/nextafter_s_xa.cpp
                           imf/intel/modf_s_xa.cpp
                           imf/intel/logb_s_xa.cpp
                           imf/intel/log2_s_ha.cpp
                           imf/intel/log1p_s_ha.cpp
                           imf/intel/log10_s_la.cpp
                           imf/intel/ln_s_ha.cpp
                           imf/intel/lgamma_s_ep.cpp
                           imf/intel/ldexp_s_xa.cpp
                           imf/intel/invcbrt_s_ha.cpp
                           imf/intel/ilogb_s_xa.cpp
                           imf/intel/tan_s_la.cpp
                           imf/intel/sin_s_ha.cpp
                           imf/intel/sinpi_s_ha.cpp
                           imf/intel/sinh_s_la.cpp
                           imf/intel/sincos_s_ha.cpp
                           imf/intel/round_s_xa.cpp
                           imf/intel/rnorm4d_s_la.cpp
                           imf/intel/rnorm3d_s_la.cpp
                           imf/intel/rhypot_s_la.cpp
                           imf/intel/remquo_s_xa.cpp
                           imf/intel/remainder_s_xa.cpp
                           imf/intel/pow_s_la.cpp
                           imf/intel/norm4d_s_la.cpp
                           imf/intel/tgamma_s_la.cpp
                           imf/intel/tanh_s_ha.cpp
                           # end INTEL_CUSTOMIZATION
                           imf/imf_inline_fp32.cpp)
set(imf_fallback_fp64_deps device.h device_imf.hpp imf_half.hpp
                           imf_utils/double_convert.cpp
                           # INTEL_CUSTOMIZATION
                           imf/_imf_include_fp64.hpp
                           imf/intel/erfinv_d_la.cpp
                           # end INTEL_CUSTOMIZATION
                           imf/imf_inline_fp64.cpp)
set(imf_fp32_fallback_src ${imf_fallback_src_dir}/imf_fp32_fallback.cpp)
set(imf_fp64_fallback_src ${imf_fallback_src_dir}/imf_fp64_fallback.cpp)
set(imf_host_cxx_flags -c
  # INTEL_CUSTOMIZATION
  -DINTEL_CUSTOMIZATION
  # end INTEL_CUSTOMIZATION
  -D__LIBDEVICE_HOST_IMPL__
)
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
                   COMMAND ${clang} ${imf_host_cxx_flags}
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
                   COMMAND ${clang} ${imf_host_cxx_flags}
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
                   COMMAND ${clang} ${imf_host_cxx_flags}
                           ${CMAKE_CURRENT_SOURCE_DIR}/imf_wrapper.cpp
                           -o ${obj_binary_dir}/imf-fp32-host.${lib-suffix}
                   MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/imf_wrapper.cpp
                   DEPENDS ${imf_obj_deps}
                   VERBATIM)

add_custom_command(OUTPUT ${obj_binary_dir}/imf-fp64-host.${lib-suffix}
                   COMMAND ${clang} ${imf_host_cxx_flags}
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
install(FILES ${spv_binary_dir}/libsycl-fallback-imf.spv
              ${spv_binary_dir}/libsycl-fallback-imf-fp64.spv
        DESTINATION ${install_dest_spv}
        COMPONENT libsycldevice)

install(FILES ${obj_binary_dir}/libsycl-fallback-imf.${lib-suffix}
              ${obj_binary_dir}/libsycl-fallback-imf-fp64.${lib-suffix}
              ${obj_binary_dir}/${devicelib_host_static}
        DESTINATION ${install_dest_lib}
        COMPONENT libsycldevice)
