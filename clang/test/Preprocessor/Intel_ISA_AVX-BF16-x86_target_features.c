#if INTEL_FEATURE_ISA_AVX_BF16
// REQUIRES: intel_feature_isa_avx_bf16
// RUN: %clang -target i386-unknown-unknown -march=atom -mavxbf16 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXBF16 %s

// AVXBF16: #define __AVX2__ 1
// AVXBF16: #define __AVXBF16__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavxbf16 -mno-avx2 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXBF16NOAVX2 %s

// AVXBF16NOAVX2-NOT: #define __AVX2__ 1
// AVXBF16NOAVX2-NOT: #define __AVXBF16__ 1

#endif // INTEL_FEATURE_ISA_AVX_BF16
