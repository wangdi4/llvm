#if INTEL_FEATURE_ISA_AVX512_NE_CONVERT
// REQUIRES: intel_feature_isa_avx512_ne_convert

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512neconvert -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVX512NECONVERT %s
// AVX512NECONVERT: #define __AVX512NECONVERT__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-avx512neconvert -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512NECONVERT %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512neconvert -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512NECONVERT %s
// NO-AVX512NECONVERT-NOT: #define __AVX512NECONVERT__ 1

#endif // INTEL_FEATURE_ISA_AVX512_NE_CONVERT