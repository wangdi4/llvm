#if INTEL_FEATURE_ISA_AVX512_RAO_INT
// REQUIRES: intel_feature_isa_avx512_rao_int

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512raoint -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVX512RAOINT %s
// AVX512RAOINT: #define __AVX512RAOINT__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-avx512raoint -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512RAOINT %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512raoint -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512RAOINT %s
// NO-AVX512RAOINT-NOT: #define __AVX512RAOINT__ 1

#endif // INTEL_FEATURE_ISA_AVX512_RAO_INT