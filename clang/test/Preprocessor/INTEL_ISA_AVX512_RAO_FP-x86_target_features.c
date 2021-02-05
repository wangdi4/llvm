#if INTEL_FEATURE_ISA_AVX512_RAO_FP
// REQUIRES: intel_feature_isa_avx512_rao_fp

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512raofp -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVX512RAOFP %s
// AVX512RAOFP: #define __AVX512RAOFP__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-avx512raofp -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512RAOFP %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512raofp -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512RAOFP %s
// NO-AVX512RAOFP-NOT: #define __AVX512RAOFP__ 1

#endif // INTEL_FEATURE_ISA_AVX512_RAO_FP