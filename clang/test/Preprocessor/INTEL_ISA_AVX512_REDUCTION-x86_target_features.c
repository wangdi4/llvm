#if INTEL_FEATURE_ISA_AVX512_REDUCTION
// REQUIRES: intel_feature_isa_avx512_reduction

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512reduction -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVX512REDUCTION %s
// AVX512REDUCTION: #define __AVX512REDUCTION__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-avx512reduction -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512REDUCTION %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512reduction -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512REDUCTION %s
// NO-AVX512REDUCTION-NOT: #define __AVX512REDUCTION__ 1

#endif // INTEL_FEATURE_ISA_AVX512_REDUCTION