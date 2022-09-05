#if INTEL_FEATURE_ISA_AVX512_MINMAX
// REQUIRES: intel_feature_isa_avx512_minmax

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512minmax -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVX512MINMAX %s
// AVX512MINMAX: #define __AVX512MINMAX__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-avx512minmax -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512MINMAX %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512minmax -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512MINMAX %s
// NO-AVX512MINMAX-NOT: #define __AVX512MINMAX__ 1

#endif // INTEL_FEATURE_ISA_AVX512_MINMAX