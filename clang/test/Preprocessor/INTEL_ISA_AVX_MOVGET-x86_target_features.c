#if INTEL_FEATURE_ISA_AVX_MOVGET
// REQUIRES: intel_feature_isa_avx_movget

// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mavxmovget -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVX-MOVGET %s
// AVX-MOVGET: #define __AVXMOVGET__ 1
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mno-avxmovget -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX-MOVGET %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mavxmovget -mno-avx2 \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX-MOVGET %s
// NO-AVX-MOVGET-NOT: #define __AVXMOVGET__ 1

#endif // INTEL_FEATURE_ISA_AVX_MOVGET
