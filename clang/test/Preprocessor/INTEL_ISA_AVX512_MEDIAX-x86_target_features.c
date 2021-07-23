#if INTEL_FEATURE_ISA_AVX512_MEDIAX
// REQUIRES: intel_feature_isa_avx512_mediax

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512mediax -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVX512MEDIAX %s
// AVX512MEDIAX: #define __AVX512MEDIAX__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-avx512mediax -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512MEDIAX %s
// NO-AVX512MEDIAX-NOT: #define __AVX512MEDIAX__ 1

#endif // INTEL_FEATURE_ISA_AVX512_MEDIAX
