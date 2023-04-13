#if INTEL_FEATURE_ISA_AVX512_NE_CONVERT_FP8
// REQUIRES: intel_feature_isa_avx512_ne_convert_fp8

// RUN: %clang -target i686-unknown-linux-gnu -mavx512neconvertfp8 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512NECONVERTFP8 %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-avx512neconvertfp8 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512NECONVERTFP8 %s
// AVX512NECONVERTFP8: "-target-feature" "+avx512neconvertfp8"
// NO-AVX512NECONVERTFP8: "-target-feature" "-avx512neconvertfp8"
#endif // INTEL_FEATURE_ISA_AVX512_NE_CONVERT_FP8
