#if INTEL_FEATURE_ISA_AVX512_REDUCTION2
// REQUIRES: intel_feature_isa_avx512_reduction2

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512reduction2 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVX512REDUCTION2 %s
// AVX512REDUCTION2: #define __AVX512REDUCTION2__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-avx512reduction2 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512REDUCTION2 %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512reduction2 -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512REDUCTION2 %s
// NO-AVX512REDUCTION2-NOT: #define __AVX512REDUCTION2__ 1

#endif // INTEL_FEATURE_ISA_AVX512_REDUCTION2