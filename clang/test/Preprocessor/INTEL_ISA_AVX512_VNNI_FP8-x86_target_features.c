#if INTEL_FEATURE_ISA_AVX512_VNNI_FP8
// REQUIRES: intel_feature_isa_avx512_vnni_fp8

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512vnnifp8 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVX512VNNIFP8 %s
// AVX512VNNIFP8: #define __AVX512VNNIFP8__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-avx512vnnifp8 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512VNNIFP8 %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512vnnifp8 -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512VNNIFP8 %s
// NO-AVX512VNNIFP8-NOT: #define __AVX512VNNIFP8__ 1

#endif // INTEL_FEATURE_ISA_AVX512_VNNI_FP8