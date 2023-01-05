#if INTEL_FEATURE_ISA_AVX512_VPMM
// REQUIRES: intel_feature_isa_avx512_vpmm

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavx512vpmm -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVX512-VPMM %s
// AVX512-VPMM: #define __AVX512VPMM__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avx512vpmm -x \
// RUN: c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512-VPMM %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavx512vpmm \
// RUN: -mno-avx512vpmm -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AVX512-VPMM %s
// NO-AVX512-VPMM-NOT: #define __AVX512-VPMM__ 1

#endif // INTEL_FEATURE_ISA_AVX512_VPMM
