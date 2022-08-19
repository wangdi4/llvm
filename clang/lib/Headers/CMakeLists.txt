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
# INTEL_CUSTOMIZATION
# INTEL_HEADERS is a list of Intel-added header files located
# in llvm/tools/clang/lib/Headers/Intel_Headers.
# We handle these files in a special way:
#   - During the build, these files are copied to the headers destination
#     directory in the build structure without creation of Intel_Headers
#     directory in the build structure.
#     For example, Intel_Headers/core/newcorefunc.h will be
#     copied into <build-headers>/core/newcorefunc.h
#   - They are installed for clang-resource-headers component without
#     creation of Intel_Headers directory.
# Such a handling allows having Intel-added header files in the repository
# with "Intel_" name in their path, as required by Xmain development process,
# and having the same files with their natural naming for compiler packages
# in the build and deploy structures
#
# For example, we add Intel_Headers/newisaintrin.h into the repository
# and add "#include <newisaintrin.h>" in immintrin.h - this will work
# due to the copying hapenning during build/install.
#
# If you have <pwd>/Intel_Headers/newheader.h, you must add it into
# INTEL_HEADERS as Intel_Headers/newheader.h
# If you have <pwd>/Intel_newheader.h, you may put it into the 'files'
# list below, or into INTEL_HEADERS as Intel_newheader.h, but in this case
# it will be copied without special handling for Intel_Headers directory.
#
# For more examples, please, read Intel_Headers/README.txt.
set(INTEL_HEADERS
  Intel_Headers/svmlintrin.h
  )
# INTEL_FEATURE_CSA
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_CSA
  Intel_Headers/csa/csacomplexfloat.h
  Intel_Headers/csa/csaintrin.h
  Intel_Headers/csa/csastream.h
  Intel_Headers/csa/x86intrin.h
  Intel_Headers/csa/bits/mathinline.h
  )
# end INTEL_FEATURE_CSA
# INTEL_FEATURE_ISA_AMX_BF8
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_BF8
  Intel_amxbf8intrin.h
  )
# end INTEL_FEATURE_ISA_AMX_BF8
# INTEL_FEATURE_ISA_AMX_MEMADVISE
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_MEMADVISE
  Intel_amxmemadviseintrin.h
  )
# end INTEL_FEATURE_ISA_AMX_MEMADVISE
# INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_MEMADVISE_EVEX
  Intel_Headers/amxmemadviseevex/amxmemadviseevexintrin.h
  )
# end INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
# INTEL_FEATURE_ISA_AMX_FUTURE
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_FUTURE
  Intel_amxfutureintrin.h
  )
# end INTEL_FEATURE_ISA_AMX_FUTURE
# INTEL_FEATURE_ISA_AMX_LNC
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_LNC
  Intel_amxlncintrin.h
  )
# end INTEL_FEATURE_ISA_AMX_LNC
# INTEL_FEATURE_ISA_AMX_FP16
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_FP16
  Intel_amxfp16intrin.h
  )
# end INTEL_FEATURE_ISA_AMX_FP16
# INTEL_FEATURE_ISA_AMX_MEMORY2
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_MEMORY2
  Intel_amxmemory2intrin.h
  )
# end INTEL_FEATURE_ISA_AMX_MEMORY2
# INTEL_FEATURE_ISA_AMX_BF16_EVEX
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_BF16_EVEX
  Intel_amxbf16evexintrin.h
  )
# end INTEL_FEATURE_ISA_AMX_BF16_EVEX
# INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_ELEMENT_EVEX
  Intel_amxelementevexintrin.h
  )
# end INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
# INTEL_FEATURE_ISA_AMX_INT8_EVEX
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_INT8_EVEX
  Intel_amxint8evexintrin.h
  )
# end INTEL_FEATURE_ISA_AMX_INT8_EVEX
# INTEL_FEATURE_ISA_AMX_TILE_EVEX
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_TILE_EVEX
  Intel_amxtileevexintrin.h
  )
# end INTEL_FEATURE_ISA_AMX_TILE_EVEX
# INTEL_FEATURE_ISA_AMX_TRANSPOSE2
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_TRANSPOSE2
  Intel_amxtranspose2intrin.h
  )
# end INTEL_FEATURE_ISA_AMX_TRANSPOSE2
# INTEL_FEATURE_ISA_AMX_CONVERT
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_CONVERT
  Intel_amxconvertintrin.h
  )
# end INTEL_FEATURE_ISA_AMX_CONVERT
# INTEL_FEATURE_ISA_AMX_TILE2
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_TILE2
  Intel_amxtile2intrin.h
  )
# end INTEL_FEATURE_ISA_AMX_TILE2
# INTEL_FEATURE_ISA_AMX_COMPLEX
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_COMPLEX
  Intel_Headers/amxcomplex/amxcomplexintrin.h
  )
# end INTEL_FEATURE_ISA_AMX_COMPLEX

# INTEL_FEATURE_ISA_AMX_TF32
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_TF32
  Intel_Headers/amxtf32/amxtf32intrin.h
  )
# end INTEL_FEATURE_ISA_AMX_TF32

# INTEL_FEATURE_ISA_AVX512_VNNI_INT8
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_VNNI_INT8
  Intel_Headers/avx512vnniint8/avx512vnniint8intrin.h
  Intel_Headers/avx512vnniint8/avx512vlvnniint8intrin.h
  )
# end INTEL_FEATURE_ISA_AVX512_VNNI_INT8
# INTEL_FEATURE_ISA_AVX512_VNNI_FP16
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_VNNI_FP16
  Intel_Headers/avx512vnnifp16/avx512vnnifp16intrin.h
  Intel_Headers/avx512vnnifp16/avx512vldotprodphpsintrin.h
  )
# end INTEL_FEATURE_ISA_AVX512_VNNI_FP16
# INTEL_FEATURE_ISA_AVX512_CONVERT
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_CONVERT
  Intel_Headers/avx512convert/avx512convertintrin.h
  Intel_Headers/avx512convert/avx512vlconvertintrin.h
  )
# end INTEL_FEATURE_ISA_AVX512_CONVERT
# INTEL_FEATURE_ISA_AVX_IFMA
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_IFMA
  Intel_Headers/avxifma/avxifmaintrin.h
  )
# end INTEL_FEATURE_ISA_AVX_IFMA
# INTEL_FEATURE_ISA_AVX_VNNI_INT8
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_VNNI_INT8
  Intel_Headers/avxvnniint8/avxvnniint8intrin.h
  )
# end INTEL_FEATURE_ISA_AVX_VNNI_INT8
# INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_DOTPROD_PHPS
  Intel_Headers/avxdotprodphps/avxdotprodphpsintrin.h
  )
# end INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
# INTEL_FEATURE_ISA_AVX_CONVERT
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_CONVERT
  Intel_Headers/avxconvert/avxconvertintrin.h
  )
# end INTEL_FEATURE_ISA_AVX_CONVERT
# INTEL_FEATURE_ISA_AVX_BF16
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_BF16
  Intel_Headers/avxbf16/avxbf16intrin.h
  )
# end INTEL_FEATURE_ISA_AVX_BF16
# INTEL_FEATURE_ISA_AVX_COMPRESS
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_COMPRESS
  Intel_Headers/avxcompress/avxcompressintrin.h
  )
# end INTEL_FEATURE_ISA_AVX_COMPRESS
# INTEL_FEATURE_ISA_AVX_MEMADVISE
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_MEMADVISE
  Intel_Headers/avxmemadvise/avxmemadviseintrin.h
  )
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_MEMADVISE
  Intel_Headers/avxmemadvise/avx512memadviseintrin.h
  Intel_Headers/avxmemadvise/avx512vlmemadviseintrin.h
  )
# end INTEL_FEATURE_ISA_AVX_MEMADVISE
#if INTEL_FEATURE_ISA_AVX512_MEDIAX
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_MEDIAX
  Intel_Headers/avx512mediax/avx512mediaxintrin.h
  )
#endif // INTEL_FEATURE_ISA_AVX512_MEDIAX
# INTEL_FEATURE_ISA_AVX_MOVGET
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_MOVGET
  Intel_Headers/avxmovget/avxmovgetintrin.h
  )
# end INTEL_FEATURE_ISA_AVX_MOVGET
# INTEL_FEATURE_ISA_AVX512_MOVGET
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_MOVGET
  Intel_Headers/avx512movget/avx512movgetintrin.h
  Intel_Headers/avx512movget/avx512vlmovgetintrin.h
  )
# end INTEL_FEATURE_ISA_AVX512_MOVGET
# INTEL_FEATURE_ISA_GPR_MOVGET
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_GPR_MOVGET
  Intel_Headers/gprmovget/gprmovgetintrin.h
  )
# end INTEL_FEATURE_ISA_GPR_MOVGET
# INTEL_FEATURE_ISA_MOVGET64B
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_MOVGET64B
  Intel_Headers/movget64b/movget64bintrin.h
  )
# end INTEL_FEATURE_ISA_MOVGET64B
# INTEL_FEATURE_ISA_RAO_INT
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_RAO_INT
  Intel_Headers/raoint/raointintrin.h
  )
# end INTEL_FEATURE_ISA_RAO_INT
# INTEL_FEATURE_ISA_AVX_RAO_INT
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_RAO_INT
  Intel_Headers/avxraoint/avxraointintrin.h
  )
# end INTEL_FEATURE_ISA_AVX_RAO_INT
# INTEL_FEATURE_ISA_AVX_RAO_FP
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_RAO_FP
  Intel_Headers/avxraofp/avxraofpintrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AVX_RAO_FP
# INTEL_FEATURE_ISA_AVX512_RAO_INT
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_RAO_INT
  Intel_Headers/avx512raoint/avx512raointintrin.h
  Intel_Headers/avx512raoint/avx512vlraointintrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AVX512_RAO_INT
# INTEL_FEATURE_ISA_AVX512_RAO_FP
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_RAO_FP
  Intel_Headers/avx512raofp/avx512raofpintrin.h
  Intel_Headers/avx512raofp/avx512vlraofpintrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AVX512_RAO_FP
# INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_AVX512_CVTROW
  Intel_Headers/amxavx512cvtrow/amxavx512cvtrowintrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
# INTEL_FEATURE_ISA_AVX_NE_CONVERT
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_NE_CONVERT
  Intel_Headers/avxneconvert/avxneconvertintrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AVX_NE_CONVERT
# INTEL_FEATURE_ISA_AVX512_NE_CONVERT
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_NE_CONVERT
  Intel_Headers/avx512neconvert/avx512neconvertintrin.h
  Intel_Headers/avx512neconvert/avx512vlneconvertintrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AVX512_NE_CONVERT
# INTEL_FEATURE_ISA_SHA512
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_SHA512
  Intel_Headers/sha512/sha512intrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_SHA512
# INTEL_FEATURE_ISA_SM3
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_SM3
  Intel_Headers/sm3/sm3intrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_SM3
# INTEL_FEATURE_ISA_SM4
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_SM4
  Intel_Headers/sm4/sm4intrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_SM4
# INTEL_FEATURE_ISA_DSPV1
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_DSPV1
  Intel_Headers/dspv1/dspv1intrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_DSPV1
# INTEL_FEATURE_ISA_AVX_VNNI_INT16
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_VNNI_INT16
  Intel_Headers/avxvnniint16/avxvnniint16intrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AVX_VNNI_INT16
# INTEL_FEATURE_ISA_AVX512_VNNI_INT16
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_VNNI_INT16
  Intel_Headers/avx512vnniint16/avx512vnniint16intrin.h
  Intel_Headers/avx512vnniint16/avx512vlvnniint16intrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AVX512_VNNI_INT16
# INTEL_FEATURE_ISA_AVX512_BF16_NE
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_BF16_NE
  Intel_Headers/avx512bf16ne/avx512bf16neintrin.h
  Intel_Headers/avx512bf16ne/avx512vlbf16neintrin.h
  )
# end INTEL_FEATURE_ISA_AVX512_BF16_NE
# INTEL_FEATURE_ISA_AMX_SPARSE
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_SPARSE
  Intel_Headers/amxsparse/amxsparseintrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AMX_SPARSE
# INTEL_FEATURE_ISA_AMX_V3
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_V3
  Intel_Headers/amxv3/amxv3intrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AMX_V3
# INTEL_FEATURE_ISA_VPINSR_VPEXTR
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_VPINSR_VPEXTR
  Intel_Headers/vpinsrvpextr/vpinsrvpextr.h
  )
# end INTEL_FEATURE_ISA_VPINSR_VPEXTR

# INTEL_FEATURE_ISA_CMPCCXADD
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_CMPCCXADD
  Intel_Headers/cmpccxadd/cmpccxaddintrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_CMPCCXADD
# INTEL_FEATURE_XUCC
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_XUCC
  Intel_Headers/pppe_intrin.h
  Intel_Headers/vt_intrin.h
  )
# end INTEL_FEATURE_XUCC
# INTEL_FEATURE_ISA_AVX512_SAT_CVT
# AUTO GENERATED BY TOOL
intel_add_file(INTEL_HEADERS
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_SAT_CVT
  Intel_Headers/avx512satcvt/avx512satcvtintrin.h
  Intel_Headers/avx512satcvt/avx512vlsatcvtintrin.h
  )
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AVX512_SAT_CVT
# end INTEL_CUSTOMIZATION

# core_files list contains the headers shared by all platforms.
# Please consider adding new platform specific headers
# to platform specific lists below.
set(core_files
  builtins.h
  float.h
  inttypes.h
  iso646.h
  limits.h
  module.modulemap
  stdalign.h
  stdarg.h
  stdatomic.h
  stdbool.h
  stddef.h
  __stddef_max_align_t.h
  stdint.h
  stdnoreturn.h
  tgmath.h
  unwind.h
  varargs.h
  )

set(arm_common_files
  # Headers shared by Arm and AArch64
  arm_acle.h
  )

set(arm_only_files
  arm_cmse.h
  armintr.h
  )

set(aarch64_only_files
  arm64intr.h
  )

set(cuda_files
  __clang_cuda_builtin_vars.h
  __clang_cuda_math.h
  __clang_cuda_cmath.h
  __clang_cuda_complex_builtins.h
  __clang_cuda_device_functions.h
  __clang_cuda_intrinsics.h
  __clang_cuda_texture_intrinsics.h
  __clang_cuda_libdevice_declares.h
  __clang_cuda_math_forward_declares.h
  __clang_cuda_runtime_wrapper.h
  )

set(hexagon_files
  hexagon_circ_brev_intrinsics.h
  hexagon_protos.h
  hexagon_types.h
  hvx_hexagon_protos.h
  )

set(hip_files
  __clang_hip_libdevice_declares.h
  __clang_hip_cmath.h
  __clang_hip_math.h
  __clang_hip_runtime_wrapper.h
  )

set(hlsl_files
  hlsl.h
  hlsl/hlsl_basic_types.h
  hlsl/hlsl_intrinsics.h
  )

set(mips_msa_files
  msa.h
  )

set(opencl_files
  opencl-c.h
  opencl-c-base.h
  )

set(ppc_files
  altivec.h
  )

set(ppc_htm_files
  htmintrin.h
  htmxlintrin.h
  )

set(systemz_files
  s390intrin.h
  vecintrin.h
  )

set(ve_files
  velintrin.h
  velintrin_gen.h
  velintrin_approx.h
  )

set(webassembly_files
  wasm_simd128.h
  )

set(x86_files
# Intrinsics
  adxintrin.h
  ammintrin.h
  amxintrin.h
  avx2intrin.h
  avx512bf16intrin.h
  avx512bitalgintrin.h
  avx512bwintrin.h
  avx512cdintrin.h
  avx512dqintrin.h
  avx512erintrin.h
  avx512fintrin.h
  avx512fp16intrin.h
  avx512ifmaintrin.h
  avx512ifmavlintrin.h
  avx512pfintrin.h
  avx512vbmi2intrin.h
  avx512vbmiintrin.h
  avx512vbmivlintrin.h
  avx512vlbf16intrin.h
  avx512vlbitalgintrin.h
  avx512vlbwintrin.h
  avx512vlcdintrin.h
  avx512vldqintrin.h
  avx512vlfp16intrin.h
  avx512vlintrin.h
  avx512vlvbmi2intrin.h
  avx512vlvnniintrin.h
  avx512vlvp2intersectintrin.h
  avx512vnniintrin.h
  avx512vp2intersectintrin.h
  avx512vpopcntdqintrin.h
  avx512vpopcntdqvlintrin.h
  avxintrin.h
  avxvnniintrin.h
  bmi2intrin.h
  bmiintrin.h
  cetintrin.h
  cldemoteintrin.h
  clflushoptintrin.h
  clwbintrin.h
  clzerointrin.h
  crc32intrin.h
  emmintrin.h
  enqcmdintrin.h
  f16cintrin.h
  fma4intrin.h
  fmaintrin.h
  fxsrintrin.h
  gfniintrin.h
  hresetintrin.h
  ia32intrin.h
  immintrin.h
  invpcidintrin.h
  keylockerintrin.h
  lwpintrin.h
  lzcntintrin.h
  mm3dnow.h
  mmintrin.h
  movdirintrin.h
  mwaitxintrin.h
  nmmintrin.h
  pconfigintrin.h
  pkuintrin.h
  pmmintrin.h
  popcntintrin.h
  prfchwintrin.h
  ptwriteintrin.h
  rdpruintrin.h
  rdseedintrin.h
  rtmintrin.h
  serializeintrin.h
  sgxintrin.h
  shaintrin.h
  smmintrin.h
  tbmintrin.h
  tmmintrin.h
  tsxldtrkintrin.h
  uintrintrin.h
  vaesintrin.h
  vpclmulqdqintrin.h
  waitpkgintrin.h
  wbnoinvdintrin.h
  __wmmintrin_aes.h
  wmmintrin.h
  __wmmintrin_pclmul.h
  x86gprintrin.h
  x86intrin.h
  xmmintrin.h
  xopintrin.h
  xsavecintrin.h
  xsaveintrin.h
  xsaveoptintrin.h
  xsavesintrin.h
  xtestintrin.h
# others
  cet.h
  cpuid.h
  )

set(windows_only_files
  intrin.h
  vadefs.h
)

set(utility_files
  mm_malloc.h
)

set(files
  ${core_files}
  ${arm_common_files}
  ${arm_only_files}
  ${aarch64_only_files}
  ${cuda_files}
  ${hexagon_files}
  ${hip_files}
  ${hlsl_files}
  ${mips_msa_files}
  ${opencl_files}
  ${ppc_files}
  ${ppc_htm_files}
  ${systemz_files}
  ${ve_files}
  ${x86_files}
  ${webassembly_files}
  ${windows_only_files}
  ${utility_files}
  )

set(cuda_wrapper_files
  cuda_wrappers/algorithm
  cuda_wrappers/complex
  cuda_wrappers/new
)

set(ppc_wrapper_files
  ppc_wrappers/mmintrin.h
  ppc_wrappers/xmmintrin.h
  ppc_wrappers/mm_malloc.h
  ppc_wrappers/emmintrin.h
  ppc_wrappers/pmmintrin.h
  ppc_wrappers/tmmintrin.h
  ppc_wrappers/smmintrin.h
  ppc_wrappers/bmiintrin.h
  ppc_wrappers/bmi2intrin.h
  ppc_wrappers/immintrin.h
  ppc_wrappers/x86intrin.h
  ppc_wrappers/x86gprintrin.h
)

set(openmp_wrapper_files
  openmp_wrappers/math.h
  openmp_wrappers/cmath
  openmp_wrappers/complex.h
  openmp_wrappers/complex
  openmp_wrappers/__clang_openmp_device_functions.h
  openmp_wrappers/complex_cmath.h
  openmp_wrappers/new
)

set(output_dir ${LLVM_LIBRARY_OUTPUT_INTDIR}/clang/${CLANG_VERSION}/include)
set(out_files)
set(generated_files)

set(arm_common_generated_files)
set(arm_only_generated_files)
set(aarch64_only_generated_files)
set(riscv_generated_files)

function(copy_header_to_output_dir src_dir file)
  set(src ${src_dir}/${file})
  set(dst ${output_dir}/${file})
  add_custom_command(OUTPUT ${dst}
    DEPENDS ${src}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${dst}
    COMMENT "Copying clang's ${file}...")
  list(APPEND out_files ${dst})
  set(out_files ${out_files} PARENT_SCOPE)
endfunction(copy_header_to_output_dir)

function(clang_generate_header td_option td_file out_file)
  clang_tablegen(${out_file} ${td_option}
  -I ${CLANG_SOURCE_DIR}/include/clang/Basic/
  SOURCE ${CLANG_SOURCE_DIR}/include/clang/Basic/${td_file})

  copy_header_to_output_dir(${CMAKE_CURRENT_BINARY_DIR} ${out_file})
  set(out_files ${out_files} PARENT_SCOPE)
  list(APPEND generated_files "${CMAKE_CURRENT_BINARY_DIR}/${out_file}")
  set(generated_files ${generated_files} PARENT_SCOPE)
endfunction(clang_generate_header)


# Copy header files from the source directory to the build directory
foreach( f ${files} ${cuda_wrapper_files} ${ppc_wrapper_files} ${openmp_wrapper_files})
  copy_header_to_output_dir(${CMAKE_CURRENT_SOURCE_DIR} ${f})
endforeach( f )

if(INTEL_CUSTOMIZATION)
  # Create build commands for Intel-added header files.
  # The commands implement copying by throwing away "Intel_Headers/" prefix.
  foreach( f ${INTEL_HEADERS} )
    string(REGEX REPLACE "^Intel_Headers/" "" fname "${f}")
    set( src ${CMAKE_CURRENT_SOURCE_DIR}/${f} )
    set( dst ${output_dir}/${fname} )
    add_custom_command(OUTPUT ${dst}
      DEPENDS ${src}
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${dst}
      COMMENT "Copying clang's ${f}...")
    install(
      FILES ${f}
      RENAME ${fname}
      COMPONENT clang-resource-headers
      PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
      DESTINATION lib${LLVM_LIBDIR_SUFFIX}/clang/${CLANG_VERSION}/include)
    list(APPEND out_files ${dst})
  endforeach( f )
endif(INTEL_CUSTOMIZATION)

# Generate header files and copy them to the build directory
if(ARM IN_LIST LLVM_TARGETS_TO_BUILD OR AArch64 IN_LIST LLVM_TARGETS_TO_BUILD)
  # Generate arm_neon.h
  clang_generate_header(-gen-arm-neon arm_neon.td arm_neon.h)
  # Generate arm_fp16.h
  clang_generate_header(-gen-arm-fp16 arm_fp16.td arm_fp16.h)
  # Generate arm_sve.h
  clang_generate_header(-gen-arm-sve-header arm_sve.td arm_sve.h)
  # Generate arm_bf16.h
  clang_generate_header(-gen-arm-bf16 arm_bf16.td arm_bf16.h)
  # Generate arm_mve.h
  clang_generate_header(-gen-arm-mve-header arm_mve.td arm_mve.h)
  # Generate arm_cde.h
  clang_generate_header(-gen-arm-cde-header arm_cde.td arm_cde.h)
  # Copy arm_neon_sve_bridge.h
  copy_header_to_output_dir(${CMAKE_CURRENT_SOURCE_DIR}
    arm_neon_sve_bridge.h
  )

  # Add headers to target specific lists
  list(APPEND arm_common_generated_files
    "${CMAKE_CURRENT_BINARY_DIR}/arm_neon.h"
    "${CMAKE_CURRENT_BINARY_DIR}/arm_fp16.h"
    )

  list(APPEND arm_only_generated_files
    "${CMAKE_CURRENT_BINARY_DIR}/arm_mve.h"
    "${CMAKE_CURRENT_BINARY_DIR}/arm_cde.h"
    )

  list(APPEND aarch64_only_generated_files
    "${CMAKE_CURRENT_BINARY_DIR}/arm_sve.h"
    "${CMAKE_CURRENT_BINARY_DIR}/arm_bf16.h"
    "${output_dir}/arm_neon_sve_bridge.h"
    )
endif()
if(RISCV IN_LIST LLVM_TARGETS_TO_BUILD)
  # Generate riscv_vector.h
  clang_generate_header(-gen-riscv-vector-header riscv_vector.td riscv_vector.h)
  list(APPEND riscv_generated_files
    "${CMAKE_CURRENT_BINARY_DIR}/riscv_vector.h"
    )
endif()


# Check if the generated headers are included in a target specific lists
# Currently, all generated headers are target specific.
set(all_target_specific_generated_files
  ${arm_common_generated_files}
  ${arm_only_generated_files}
  ${aarch64_only_generated_files}
  ${riscv_generated_files})
foreach( f ${generated_files} )
  if (NOT ${f} IN_LIST all_target_specific_generated_files)
    message(WARNING "${f} is a generated header but it is not included in any "
                    "target specific header lists! The resource-headers "
                    "distribtion target will be incorrect!")
  endif()
endforeach( f )

function(add_header_target target_name file_list)
  add_custom_target(${target_name} DEPENDS ${file_list})
  set_target_properties(${target_name} PROPERTIES
    FOLDER "Misc"
    RUNTIME_OUTPUT_DIRECTORY "${output_dir}")
endfunction()

# The catch-all clang-resource-headers target
add_custom_target("clang-resource-headers" ALL DEPENDS ${out_files})
set_target_properties("clang-resource-headers" PROPERTIES
  FOLDER "Misc"
  RUNTIME_OUTPUT_DIRECTORY "${output_dir}")
add_dependencies("clang-resource-headers"
                 "core-resource-headers"
                 "arm-common-resource-headers"
                 "arm-resource-headers"
                 "aarch64-resource-headers"
                 "cuda-resource-headers"
                 "hexagon-resource-headers"
                 "hip-resource-headers"
                 "hlsl-resource-headers"
                 "mips-resource-headers"
                 "ppc-resource-headers"
                 "ppc-htm-resource-headers"
                 "riscv-resource-headers"
                 "systemz-resource-headers"
                 "ve-resource-headers"
                 "webassembly-resource-headers"
                 "x86-resource-headers"
                 "opencl-resource-headers"
                 "openmp-resource-headers"
                 "windows-resource-headers"
                 "utility-resource-headers")

# Core/common headers
add_header_target("core-resource-headers" ${core_files})
add_header_target("arm-common-resource-headers" "${arm_common_files};${arm_common_generated_files}")

# Architecture/platform specific targets
add_header_target("arm-resource-headers" "${arm_only_files};${arm_only_generated_files}")
add_header_target("aarch64-resource-headers" "${aarch64_only_files};${aarch64_only_generated_files}")
add_header_target("cuda-resource-headers" "${cuda_files};${cuda_wrapper_files}")
add_header_target("hexagon-resource-headers" "${hexagon_files}")
add_header_target("hip-resource-headers" "${hip_files}")
add_header_target("mips-resource-headers" "${mips_msa_files}")
add_header_target("ppc-resource-headers" "${ppc_files};${ppc_wrapper_files}")
add_header_target("ppc-htm-resource-headers" "${ppc_htm_files}")
add_header_target("riscv-resource-headers" "${riscv_generated_files}")
add_header_target("systemz-resource-headers" "${systemz_files}")
add_header_target("ve-resource-headers" "${ve_files}")
add_header_target("webassembly-resource-headers" "${webassembly_files}")
add_header_target("x86-resource-headers" "${x86_files}")

# Other header groupings
add_header_target("hlsl-resource-headers" ${hlsl_files})
add_header_target("opencl-resource-headers" ${opencl_files})
add_header_target("openmp-resource-headers" ${openmp_wrapper_files})
add_header_target("windows-resource-headers" ${windows_only_files})
add_header_target("utility-resource-headers" ${utility_files})

set(header_install_dir ${CMAKE_INSTALL_LIBDIR}/clang/${CLANG_VERSION}/include)

if(INTEL_CUSTOMIZATION)
  # Avoid deploying header files for unsupported targets.
  list(REMOVE_ITEM files
    # ARM
    arm_acle.h
    arm_cmse.h
    armintr.h
    arm64intr.h

    # MIPS
    msa.h

    # SystemZ, PowerPC
    altivec.h
    s390intrin.h
    htmintrin.h
    htmxlintrin.h
    vecintrin.h

    # CUDA
    # Note that openmp_wrappers depend on these files.
    # We exclude openmp_wrappers below.
    __clang_cuda_builtin_vars.h
    __clang_cuda_cmath.h
    __clang_cuda_complex_builtins.h
    __clang_cuda_device_functions.h
    __clang_cuda_intrinsics.h
    __clang_cuda_libdevice_declares.h
    __clang_cuda_math_forward_declares.h
    __clang_cuda_runtime_wrapper.h
    )
  # Currently all generated files are for ARM.  Avoid deploying them.
  set(generated_files)
endif(INTEL_CUSTOMIZATION)

#############################################################
# Install rules for the catch-all clang-resource-headers target
install(
  FILES ${files} ${generated_files}
  DESTINATION ${header_install_dir}
  COMPONENT clang-resource-headers)

if(NOT INTEL_CUSTOMIZATION)
# Avoid deploying CUDA/NVPTX and PPC files.
install(
  FILES ${cuda_wrapper_files}
  DESTINATION ${header_install_dir}/cuda_wrappers
  COMPONENT clang-resource-headers)

install(
  FILES ${ppc_wrapper_files}
  DESTINATION ${header_install_dir}/ppc_wrappers
  COMPONENT clang-resource-headers)

install(
  FILES ${openmp_wrapper_files}
  DESTINATION ${header_install_dir}/openmp_wrappers
  COMPONENT clang-resource-headers)
endif(NOT INTEL_CUSTOMIZATION)

#############################################################
# Install rules for separate header lists
install(
  FILES ${core_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT core-resource-headers)

install(
  FILES ${arm_common_files} ${arm_common_generated_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT arm-common-resource-headers)

install(
  FILES ${arm_only_files} ${arm_only_generated_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT arm-resource-headers)

install(
  FILES ${aarch64_only_files} ${aarch64_only_generated_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT aarch64-resource-headers)

install(
  FILES ${cuda_wrapper_files}
  DESTINATION ${header_install_dir}/cuda_wrappers
  EXCLUDE_FROM_ALL
  COMPONENT cuda-resource-headers)

install(
  FILES ${cuda_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT cuda-resource-headers)

install(
  FILES ${hexagon_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT hexagon-resource-headers)

install(
  FILES ${hip_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT hip-resource-headers)

install(
  FILES ${mips_msa_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT mips-resource-headers)

install(
  FILES ${ppc_wrapper_files}
  DESTINATION ${header_install_dir}/ppc_wrappers
  EXCLUDE_FROM_ALL
  COMPONENT ppc-resource-headers)

install(
  FILES ${ppc_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT ppc-resource-headers)

install(
  FILES ${ppc_htm_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT ppc-htm-resource-headers)

install(
  FILES ${riscv_generated_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT riscv-resource-headers)

install(
  FILES ${systemz_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT systemz-resource-headers)

install(
  FILES ${ve_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT ve-resource-headers)

install(
  FILES ${webassembly_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT webassembly-resource-headers)

install(
  FILES ${x86_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT x86-resource-headers)

install(
  FILES ${hlsl_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT hlsl-resource-headers)

install(
  FILES ${opencl_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT opencl-resource-headers)

install(
  FILES ${openmp_wrapper_files}
  DESTINATION ${header_install_dir}/openmp_wrappers
  EXCLUDE_FROM_ALL
  COMPONENT openmp-resource-headers)

install(
  FILES ${utility_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT utility-resource-headers)

install(
  FILES ${windows_only_files}
  DESTINATION ${header_install_dir}
  EXCLUDE_FROM_ALL
  COMPONENT windows-resource-headers)
#############################################################

if (NOT LLVM_ENABLE_IDE)
  add_llvm_install_targets(install-clang-resource-headers
                           DEPENDS clang-resource-headers
                           COMPONENT clang-resource-headers)

  add_llvm_install_targets(install-core-resource-headers
                           DEPENDS core-resource-headers
                           COMPONENT core-resource-headers)
  add_llvm_install_targets(install-arm-common-resource-headers
                           DEPENDS arm-common-resource-headers
                           COMPONENT arm-common-resource-headers)
  add_llvm_install_targets(install-arm-resource-headers
                           DEPENDS arm-resource-headers
                           COMPONENT arm-resource-headers)
  add_llvm_install_targets(install-aarch64-resource-headers
                           DEPENDS aarch64-resource-headers
                           COMPONENT aarch64-resource-headers)
  add_llvm_install_targets(install-cuda-resource-headers
                           DEPENDS cuda-resource-headers
                           COMPONENT cuda-resource-headers)
  add_llvm_install_targets(install-hexagon-resource-headers
                           DEPENDS hexagon-resource-headers
                           COMPONENT hexagon-resource-headers)
  add_llvm_install_targets(install-hip-resource-headers
                           DEPENDS hip-resource-headers
                           COMPONENT hip-resource-headers)
  add_llvm_install_targets(install-mips-resource-headers
                           DEPENDS mips-resource-headers
                           COMPONENT mips-resource-headers)
  add_llvm_install_targets(install-ppc-resource-headers
                           DEPENDS ppc-resource-headers
                           COMPONENT ppc-resource-headers)
  add_llvm_install_targets(install-ppc-htm-resource-headers
                           DEPENDS ppc-htm-resource-headers
                           COMPONENT ppc-htm-resource-headers)
  add_llvm_install_targets(install-riscv-resource-headers
                           DEPENDS riscv-resource-headers
                           COMPONENT riscv-resource-headers)
  add_llvm_install_targets(install-systemz-resource-headers
                           DEPENDS systemz-resource-headers
                           COMPONENT systemz-resource-headers)
  add_llvm_install_targets(install-ve-resource-headers
                           DEPENDS ve-resource-headers
                           COMPONENT ve-resource-headers)
  add_llvm_install_targets(install-x86-resource-headers
                           DEPENDS x86-resource-headers
                           COMPONENT x86-resource-headers)
  add_llvm_install_targets(install-webassembly-resource-headers
                           DEPENDS webassembly-resource-headers
                           COMPONENT webassembly-resource-headers)

  add_llvm_install_targets(install-hlsl-resource-headers
                           DEPENDS hlsl-resource-headers
                           COMPONENT hlsl-resource-headers)
  add_llvm_install_targets(install-opencl-resource-headers
                           DEPENDS opencl-resource-headers
                           COMPONENT opencl-resource-headers)
  add_llvm_install_targets(install-openmp-resource-headers
                           DEPENDS openmp-resource-headers
                           COMPONENT openmp-resource-headers)
  add_llvm_install_targets(install-windows-resource-headers
                           DEPENDS windows-resource-headers
                           COMPONENT windows-resource-headers)
  add_llvm_install_targets(install-utility-resource-headers
                           DEPENDS utility-resource-headers
                           COMPONENT utility-resource-headers)
endif()
