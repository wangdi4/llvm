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
# INTEL_COLLAB
set(binary_dir "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
if (WIN32)
  set(omp_compile_opts -Qiopenmp --driver-mode=cl)
  set(omp_target_opts -Qopenmp-targets=spir64_gen,spir64_x86_64,spir64)
  set(objext .obj)
  set(cmplr_obj_out -c -Fo)
else()
  set(omp_compile_opts -fiopenmp)
  set(omp_target_opts -fopenmp-targets=spir64_gen,spir64_x86_64,spir64)
  set(objext .o)
  set(cmplr_obj_out -c -o)
endif()

# INTEL_CUSTOMIZATION
# Temporary workaround for CMPLRLLVM-29069:
list(APPEND omp_compile_opts -mllvm -disable-vector-combine)
# end INTEL_CUSTOMIZATION

# Specify O1 optimizations to avoid noinline attribute
# in SPIR-V code. Device compilers have to be able to inline
# libdevice functions.
set(clang_opts -fdeclare-spirv-builtins -O1)
if (INTEL_CUSTOMIZATION)
  # FIXME: clang has to work with -fiopenmp -fopenmp-targets=spir64
  #        the same way icx does.
  list(APPEND omp_compile_opts --intel)
  set(new_clang_opts)
  foreach(o ${clang_opts})
    list(APPEND new_clang_opts -Xclang ${o})
  endforeach()
  set(clang_opts ${new_clang_opts})
endif(INTEL_CUSTOMIZATION)
list(APPEND omp_compile_opts
  -DOMP_LIBDEVICE
  -DINTEL_COLLAB
  # Disable warnings for the host compilation, where
  # we declare all functions as 'static'.
  -Wno-undefined-internal
  )

if (INTEL_CUSTOMIZATION)
  list(APPEND omp_compile_opts
    -DINTEL_CUSTOMIZATION
    )
endif(INTEL_CUSTOMIZATION)

set(omplib_objs)
set(omplib_spvs)

function(add_obj_file src dst)
  cmake_parse_arguments(ARG "" "" "DEPENDS" ${ARGN})

  set(cmplr $<TARGET_FILE:clang>)

  if (WIN32)
    # Build object files with /Zl on Windows to get rid
    # of default libraries dependency. Anyway, the object
    # files are empty for the host part.
    # Note that /Zl only works in 'cl' driver mode.
    #
    # We cannot use check_c[xx]_compiler_flags() functions for /Zl,
    # because the linker fail for the test example compiled with /Zl.
    # Try to set /Zl unconditionally for the time being.
    #   check_c_compiler_flag("/Zl" LIBDEVICE_C_SUPPORTS_ZL)
    #   check_cxx_compiler_flag("/Zl" LIBDEVICE_CXX_SUPPORTS_ZL)
    set(LIBDEVICE_C_SUPPORTS_ZL TRUE)
    set(LIBDEVICE_CXX_SUPPORTS_ZL TRUE)
    if (LIBDEVICE_C_SUPPORTS_ZL AND LIBDEVICE_CXX_SUPPORTS_ZL)
      list(APPEND omp_compile_opts /Zl)
    endif()
    # Avoid linker directives like this in the resulting object files:
    #   /FAILIFMISMATCH:RuntimeLibrary=MT_StaticRelease
    list(APPEND omp_compile_opts -D_ALLOW_RUNTIME_LIBRARY_MISMATCH)
  endif(WIN32)

  add_custom_command(OUTPUT ${dst}
    COMMAND ${cmplr} ${omp_compile_opts} ${clang_opts}
            ${omp_target_opts}
            ${src} ${cmplr_obj_out}${dst}
    MAIN_DEPENDENCY ${src}
    DEPENDS ${ARG_DEPENDS}
            clang clang-offload-bundler
    )

  list(APPEND omplib_objs ${dst})
  set(omplib_objs ${omplib_objs} PARENT_SCOPE)
endfunction()

function(add_spv_file src dst)
  cmake_parse_arguments(ARG "" "" "DEPENDS" ${ARGN})

  set(cmplr $<TARGET_FILE:clang>)
  set(llvm_spirv $<TARGET_FILE:llvm-spirv>)
  set(clang_offload_bundler $<TARGET_FILE:clang-offload-bundler>)

  add_custom_command(OUTPUT ${dst}
    COMMAND ${cmplr} ${omp_compile_opts} ${clang_opts}
            ${omp_target_opts}
            ${src} ${cmplr_obj_out}${dst}${objext}
    COMMAND ${clang_offload_bundler} -type=o -unbundle
            -targets=openmp-spir64
            -input=${dst}${objext} -output=${dst}.target.bc
    COMMAND ${llvm_spirv} -spirv-ext=+all ${dst}.target.bc -o ${dst}
    MAIN_DEPENDENCY ${src}
    DEPENDS ${ARG_DEPENDS}
            clang llvm-spirv clang-offload-bundler
    )

  list(APPEND omplib_spvs ${dst})
  set(omplib_spvs ${omplib_spvs} PARENT_SCOPE)
endfunction()

# Standard functionality.
if (WIN32)
  add_obj_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/crt_wrapper.cpp
    ${binary_dir}/libomp-msvc${objext}
    DEPENDS wrapper.h device.h)
else(WIN32)
  add_obj_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/crt_wrapper.cpp
    ${binary_dir}/libomp-glibc${objext}
    DEPENDS wrapper.h device.h)
endif(WIN32)
add_obj_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/fallback-cassert.cpp
  ${binary_dir}/libomp-fallback-cassert${objext}
  DEPENDS wrapper.h device.h
  )
add_spv_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/fallback-cassert.cpp
  ${binary_dir}/libomp-fallback-cassert.spv
  DEPENDS wrapper.h device.h
  )
add_obj_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/fallback-cstring.cpp
  ${binary_dir}/libomp-fallback-cstring${objext}
  DEPENDS wrapper.h device.h
  )
add_spv_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/fallback-cstring.cpp
  ${binary_dir}/libomp-fallback-cstring.spv
  DEPENDS wrapper.h device.h
  )

# Standard math.
add_obj_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmath_wrapper.cpp
  ${binary_dir}/libomp-cmath${objext}
  DEPENDS device_math.h device.h
  )
add_obj_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/fallback-cmath.cpp
  ${binary_dir}/libomp-fallback-cmath${objext}
  DEPENDS device_math.h device.h
  )
add_spv_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/fallback-cmath.cpp
  ${binary_dir}/libomp-fallback-cmath.spv
  DEPENDS device_math.h device.h
  )
add_obj_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmath_wrapper_fp64.cpp
  ${binary_dir}/libomp-cmath-fp64${objext}
  DEPENDS device_math.h device.h
  )
add_obj_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/fallback-cmath-fp64.cpp
  ${binary_dir}/libomp-fallback-cmath-fp64${objext}
  DEPENDS device_math.h device.h
  )
add_spv_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/fallback-cmath-fp64.cpp
  ${binary_dir}/libomp-fallback-cmath-fp64.spv
  DEPENDS device_math.h device.h
  )

# Complex data types math.
add_obj_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/complex_wrapper.cpp
  ${binary_dir}/libomp-complex${objext}
  DEPENDS device_complex.h device.h
  )
add_obj_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/fallback-complex.cpp
  ${binary_dir}/libomp-fallback-complex${objext}
  DEPENDS device_math.h device_complex.h device.h
  )
add_spv_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/fallback-complex.cpp
  ${binary_dir}/libomp-fallback-complex.spv
  DEPENDS device_math.h device_complex.h device.h
  )
add_obj_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/complex_wrapper_fp64.cpp
  ${binary_dir}/libomp-complex-fp64${objext}
  DEPENDS device_complex.h device.h
  )
add_obj_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/fallback-complex-fp64.cpp
  ${binary_dir}/libomp-fallback-complex-fp64${objext}
  DEPENDS device_math.h device_complex.h device.h
  )
add_spv_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/fallback-complex-fp64.cpp
  ${binary_dir}/libomp-fallback-complex-fp64.spv
  DEPENDS device_math.h device_complex.h device.h
  )

# ITT APIs
add_obj_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/itt_stubs.cpp
  ${binary_dir}/libomp-itt-stubs${objext}
  DEPENDS device_itt.h spirv_vars.h device.h
  )
add_obj_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/itt_user_wrappers.cpp
  ${binary_dir}/libomp-itt-user-wrappers${objext}
  DEPENDS device_itt.h spirv_vars.h device.h
  )
add_obj_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/itt_compiler_wrappers.cpp
  ${binary_dir}/libomp-itt-compiler-wrappers${objext}
  DEPENDS device_itt.h spirv_vars.h device.h
  )

# INTEL_CUSTOMIZATION
include(OMPImfDevice)
# end INTEL_CUSTOMIZATION

add_custom_target(libompdevice-obj DEPENDS ${omplib_objs})

add_custom_target(libompdevice-spv DEPENDS ${omplib_spvs})

add_custom_target(libompdevice DEPENDS libompdevice-obj libompdevice-spv)

set(install_dest lib${LLVM_LIBDIR_SUFFIX})

install(FILES ${omplib_objs}
        DESTINATION ${install_dest}
        COMPONENT libompdevice)

if (WIN32)
  set(install_dest bin)
endif(WIN32)

install(FILES ${omplib_spvs}
        DESTINATION ${install_dest}
        COMPONENT libompdevice)
# end INTEL_COLLAB
