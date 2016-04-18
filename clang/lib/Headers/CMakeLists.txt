# INTEL_CUSTOMIZATION
# This file has been heavily modified. Merge with care.
set(modified_files
  adxintrin.h
  avx2intrin.h
  avx512bwintrin.h
  avx512cdintrin.h
  avx512erintrin.h
  avx512fintrin.h
  avx512vlbwintrin.h
  avx512vlintrin.h
  avx512dqintrin.h
  avx512vldqintrin.h
  pkuintrin.h
  avxintrin.h
  bmi2intrin.h
  bmiintrin.h
  emmintrin.h
  f16cintrin.h
  fmaintrin.h
  fxsrintrin.h
  ia32intrin.h
  immintrin.h
  lzcntintrin.h
  mmintrin.h
  mm_malloc.h
  nmmintrin.h
  pmmintrin.h
  popcntintrin.h
  prfchwintrin.h
  rdseedintrin.h
  rtmintrin.h
  shaintrin.h
  smmintrin.h
  tmmintrin.h
  __wmmintrin_aes.h
  wmmintrin.h
  __wmmintrin_pclmul.h
  xtestintrin.h
  x86intrin.h
  xsaveintrin.h
  xsaveoptintrin.h
  xsavecintrin.h
  xsavesintrin.h  
  xmmintrin.h
  )

set(special_files
  __x86intrin_features.h
  Intrin.h
  )

set(orig_files
  altivec.h
  ammintrin.h
  arm_acle.h
  __clang_cuda_cmath.h
  __clang_cuda_runtime_wrapper.h
  cpuid.h
  cuda_builtin_vars.h
  float.h
  fma4intrin.h
  htmintrin.h
  htmxlintrin.h
  inttypes.h
  iso646.h
  limits.h
  module.modulemap
  mm3dnow.h  
  s390intrin.h
  stdalign.h
  stdarg.h
  stdatomic.h
  stdbool.h
  stddef.h
  __stddef_max_align_t.h
  stdint.h
  stdnoreturn.h
  tbmintrin.h
  tgmath.h
  unwind.h
  vadefs.h
  varargs.h
  vecintrin.h
  xopintrin.h
  xtestintrin.h
  )

set(intel_files ${modified_files} ${special_files})
set(all_files ${orig_files} ${modified_files} ${special_files})
  
set(output_dir ${LLVM_LIBRARY_OUTPUT_INTDIR}/clang/${CLANG_VERSION}/include)
set(intel_output_dir ${LLVM_INCLUDE_DIR}/intel)

# Generate arm_neon.h
clang_tablegen(arm_neon.h -gen-arm-neon
  SOURCE ${CLANG_SOURCE_DIR}/include/clang/Basic/arm_neon.td)

set(out_files)
foreach( f ${all_files} )
  set( src ${CMAKE_CURRENT_SOURCE_DIR}/${f} )
  set( dst ${output_dir}/${f} )
  add_custom_command(OUTPUT ${dst}
    DEPENDS ${src}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${dst}
    COMMENT "Copying clang's ${f}...")
  list(APPEND out_files ${dst})
endforeach( f )

add_custom_command(OUTPUT ${output_dir}/arm_neon.h 
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/arm_neon.h
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/arm_neon.h ${output_dir}/arm_neon.h
  COMMENT "Copying clang's arm_neon.h...")
list(APPEND out_files ${output_dir}/arm_neon.h)

add_custom_target(clang-headers ALL DEPENDS ${out_files})
set_target_properties(clang-headers PROPERTIES FOLDER "Misc")

add_custom_command(TARGET clang-headers
  POST_BUILD
  COMMAND ${PYTHON_EXECUTABLE} ${LLVM_MAIN_SRC_DIR}/tools/clang/utils/Intel/gen_assume.py ${LLVM_MAIN_SRC_DIR}/tools/clang/utils/Intel/x86_intrin.xml ${output_dir} ${modified_files}
  COMMENT "Adding intrinsic assume() builtins")

set(intel_out_files)
foreach( f ${intel_files} )
  set( src ${output_dir}/${f} )
  set( dst ${intel_output_dir}/${f} )
  add_custom_command(OUTPUT ${dst}
    DEPENDS ${src} clang-headers
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${dst}
    COMMENT "Copying ${f} with assumes...")
  list(APPEND intel_out_files ${dst})
endforeach( f )
  
add_custom_target(intel-intrinsic-headers ALL DEPENDS ${intel_out_files})
set_target_properties(intel-intrinsic-headers PROPERTIES FOLDER "Misc")

if (MSVC)
  foreach( f ${out_files} )
    string(REPLACE "${CMAKE_CFG_INTDIR}" "\${BUILD_TYPE}" install_file ${f})
    list(APPEND install_headers ${install_file})
  endforeach( f )
else()
  set(install_headers out_files)
endif()

install(
  FILES ${install_headers} ${CMAKE_CURRENT_BINARY_DIR}/arm_neon.h
  COMPONENT clang-headers
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  DESTINATION lib${LLVM_LIBDIR_SUFFIX}/clang/${CLANG_VERSION}/include)

if (NOT CMAKE_CONFIGURATION_TYPES) # don't add this for IDE's.
  add_custom_target(install-clang-headers
    DEPENDS clang-headers
    COMMAND "${CMAKE_COMMAND}"
            -DCMAKE_INSTALL_COMPONENT=clang-headers
            -P "${CMAKE_BINARY_DIR}/cmake_install.cmake")
endif()
