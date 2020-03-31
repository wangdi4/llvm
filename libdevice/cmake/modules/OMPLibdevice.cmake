# INTEL_COLLAB
if (WIN32)
  # FIXME: resolve mangling issues on Windows.
  add_custom_target(libompdevice)
  return()
endif()

set(binary_dir "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
if (WIN32)
  set(omp_compile_opts
    -Qiopenmp -Qopenmp-targets=spir64 -c
    )
  set(objext .obj)
  set(cmplr_obj_out -Fo)
else()
  set(omp_compile_opts
    -fiopenmp -fopenmp-targets=spir64 -c
    )
  set(objext .o)
  set(cmplr_obj_out -o)
endif()

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

  if (INTEL_CUSTOMIZATION)
    # FIXME: clang has to work with -fiopenmp -fopenmp-targets=spir64
    #        the same way icx does.
    set(cmplr $<TARGET_FILE:icx>)
  else(INTEL_CUSTOMIZATION)
    set(cmplr $<TARGET_FILE:clang>)
  endif(INTEL_CUSTOMIZATION)

  add_custom_command(OUTPUT ${dst}
                     COMMAND ${cmplr} ${omp_compile_opts}
                             ${src} ${cmplr_obj_out}${dst}
                     MAIN_DEPENDENCY ${src}
                     DEPENDS device_complex.h device.h
                             ${ARG_DEPENDS}
                             clang clang-offload-bundler
# INTEL_CUSTOMIZATION
                             icx
# end INTEL_CUSTOMIZATION
                     VERBATIM)

  list(APPEND omplib_objs ${dst})
  set(omplib_objs ${omplib_objs} PARENT_SCOPE)
endfunction()

function(add_spv_file src dst)
  cmake_parse_arguments(ARG "" "" "DEPENDS" ${ARGN})

  if (INTEL_CUSTOMIZATION)
    # FIXME: clang has to work with -fiopenmp -fopenmp-targets=spir64
    #        the same way icx does.
    set(cmplr $<TARGET_FILE:icx>)
  else(INTEL_CUSTOMIZATION)
    set(cmplr $<TARGET_FILE:clang>)
  endif(INTEL_CUSTOMIZATION)
  set(llvm_spirv $<TARGET_FILE:llvm-spirv>)
  set(clang_offload_bundler $<TARGET_FILE:clang-offload-bundler>)

  add_custom_command(OUTPUT ${dst}
                     COMMAND ${cmplr} ${omp_compile_opts}
                             ${src} ${cmplr_obj_out}${dst}${objext}
                     COMMAND ${clang_offload_bundler} -type=o -unbundle
                             -targets=openmp-spir64
                             -inputs=${dst}${objext} -outputs=${dst}.target.bc
                     COMMAND ${llvm_spirv} -spirv-ext=+all ${dst}.target.bc
                             -o ${dst}
                     MAIN_DEPENDENCY ${src}
                     DEPENDS ${ARG_DEPENDS}
                             clang llvm-spirv clang-offload-bundler
# INTEL_CUSTOMIZATION
                             icx
# end INTEL_CUSTOMIZATION
                     VERBATIM)

  list(APPEND omplib_spvs ${dst})
  set(omplib_spvs ${omplib_spvs} PARENT_SCOPE)
endfunction()

# Standard functionality.
if (WIN32)
  add_obj_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/msvc_wrapper.cpp
    ${binary_dir}/libomp-msvc${objext}
    DEPENDS wrapper.h device.h)
else(WIN32)
  add_obj_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/glibc_wrapper.cpp
    ${binary_dir}/libomp-glibc${objext}
    DEPENDS wrapper.h device.h)
endif(WIN32)
# FIXME: resolve assertion in clang.
#add_spv_file(
#  ${CMAKE_CURRENT_SOURCE_DIR}/fallback-cassert.cpp
#  ${binary_dir}/libomp-fallback-cassert.spv
#  DEPENDS wrapper.h device.h
#  )

# Standard math.
add_obj_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmath_wrapper.cpp
  ${binary_dir}/libomp-cmath${objext}
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
add_spv_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/fallback-complex-fp64.cpp
  ${binary_dir}/libomp-fallback-complex-fp64.spv
  DEPENDS device_math.h device_complex.h device.h
  )

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
