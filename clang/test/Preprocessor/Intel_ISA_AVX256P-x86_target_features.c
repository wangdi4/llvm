#if INTEL_FEATURE_ISA_AVX256P
// REQUIRES: intel_feature_isa_avx256p

// RUN: %clang -target x86_64-unknown-linux-gnu -mavx256p -x c -E -dM -o - %s | FileCheck -check-prefix=AVX256P %s
// AVX256P: #define __AVX256P__ 1
// RUN: %clang -target x86_64-unknown-linux-gnu -mno-avx256p -x c -E -dM -o - %s | FileCheck -check-prefix=NO-AVX256P %s
// NO-AVX256P-NOT: __AVX256P__
#endif //INTEL_FEATURE_ISA_AVX256P
