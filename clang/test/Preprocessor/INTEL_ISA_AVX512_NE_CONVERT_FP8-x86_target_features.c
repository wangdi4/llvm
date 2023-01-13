#if INTEL_FEATURE_ISA_AVX512_NE_CONVERT_FP8
// REQUIRES: intel_feature_isa_avx512_ne_convert_fp8

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512neconvertfp8 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVX512NECONVERTFP8 %s
// AVX512NECONVERTFP8: #define __AVX512NECONVERTFP8__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-avx512neconvertfp8 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512NECONVERTFP8 %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512neconvertfp8 -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512NECONVERTFP8 %s
// NO-AVX512NECONVERTFP8-NOT: #define __AVX512NECONVERTFP8__ 1

#endif // INTEL_FEATURE_ISA_AVX512_NE_CONVERT_FP8
