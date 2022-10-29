
function(add_obj_file srcs)
  cmake_parse_arguments(ARG "" "" "DEPENDS" ${ARGN})

  set(cmplr $<TARGET_FILE:clang>)
  set(clang_offload_bundler $<TARGET_FILE:clang-offload-bundler>)

  # Math functions must not be compiled with fast-math themselves
  if (WIN32)
    set(target_opts -Qopenmp-targets=spir64,spir64_gen=\"-fp-model=precise -fno-math-errno\" -Qopenmp-target-simd -Qopenmp-target-intel-proprietary-opts)
  else()
    set(target_opts -fopenmp-targets=spir64,spir64_gen=\"-fp-model=precise -fno-math-errno -mllvm -vpo-paropt-preserve-llvm-intrin\" -fopenmp-target-simd -fopenmp-target-intel-proprietary-opts)
  endif()

  set(clang_opts -I ${CMAKE_CURRENT_SOURCE_DIR}/imf -Xclang -fdeclare-spirv-builtins -Xclang -O2 -Xclang -fopenmp-target-declare-simd-spir -D__SPIR__)

  set(fname libomp-device-svml)
  set(imf_lib ${fname}${objext})

  add_custom_command(OUTPUT ${binary_dir}/${imf_lib}
    COMMAND ${cmplr} ${omp_compile_opts} ${clang_opts}
            ${target_opts}
            ${srcs} -c -o ${binary_dir}/${imf_lib}
    MAIN_DEPENDENCY ${srcs}
    DEPENDS ${ARG_DEPENDS}
            clang clang-offload-bundler
    COMMENT ""
    )
  set(imf_lib ${binary_dir}/${imf_lib} PARENT_SCOPE)
endfunction()

set(imf_src_dir ${CMAKE_CURRENT_SOURCE_DIR})
set(imf_fallback_src_dir ${obj_binary_dir}/libdevice)
set(imf_simd_src ${imf_fallback_src_dir}/imf_simd.cpp)

add_custom_command(OUTPUT ${imf_simd_src}
                   COMMAND ${CMAKE_COMMAND} -D SRC_DIR=${imf_src_dir}
                                            -D DEST_DIR=${imf_fallback_src_dir}
                                            -D OMP_LIBDEVICE=1
                                            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/ImfSrcConcate.cmake
                   )

set(imf_lib)
add_obj_file("${imf_simd_src}")

list(APPEND omplib_objs ${imf_lib})
set(omplib_objs ${omplib_objs} PARENT_SCOPE)
