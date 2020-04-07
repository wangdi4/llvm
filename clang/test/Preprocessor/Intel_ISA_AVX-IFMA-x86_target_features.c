#if INTEL_FEATURE_ISA_AVX_IFMA
// REQUIRES: intel_feature_isa_avx_ifma
// RUN: %clang -target i386-unknown-unknown -march=atom -mavxifma -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXIFMA %s

// AVXIFMA: #define __AVX2__ 1
// AVXIFMA: #define __AVXIFMA__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavxifma -mno-avx2 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXIFMANOAVX2 %s

// AVXIFMANOAVX2-NOT: #define __AVX2__ 1
// AVXIFMANOAVX2-NOT: #define __AVXIFMA__ 1

#endif // INTEL_FEATURE_ISA_AVX_IFMA
