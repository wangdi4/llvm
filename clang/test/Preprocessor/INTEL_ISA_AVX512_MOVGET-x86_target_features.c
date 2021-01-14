#if INTEL_FEATURE_ISA_AVX512_MOVGET
// REQUIRES: intel_feature_isa_avx512_movget

// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mavx512movget -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVX512-MOVGET %s
// AVX512-MOVGET: #define __AVX512MOVGET__ 1
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mno-avx512movget -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512-MOVGET %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mavx512movget -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512-MOVGET %s
// NO-AVX512-MOVGET-NOT: #define __AVX512MOVGET__ 1

#endif // INTEL_FEATURE_ISA_AVX512_MOVGET
