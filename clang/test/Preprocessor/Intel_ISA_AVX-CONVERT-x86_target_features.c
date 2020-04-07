#if INTEL_FEATURE_ISA_AVX_CONVERT
// REQUIRES: intel_feature_isa_avx_convert
// RUN: %clang -target i386-unknown-unknown -march=atom -mavxconvert -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXCONVERT %s

// AVXCONVERT: #define __AVX2__ 1
// AVXCONVERT: #define __AVXCONVERT__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mno-avxconvert -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=NOAVXCONVERT %s

// NOAVXCONVERT-NOT: #define __AVXCONVERT__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavxconvert -mno-avx2 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXCONVERTNOAVX2 %s

// AVXCONVERTNOAVX2-NOT: #define __AVX2__ 1
// AVXCONVERTNOAVX2-NOT: #define __AVXCONVERT__ 1

#endif // INTEL_FEATURE_ISA_AVX_CONVERT
