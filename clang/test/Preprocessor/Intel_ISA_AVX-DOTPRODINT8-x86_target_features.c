#if INTEL_FEATURE_ISA_AVX_DOTPROD_INT8
// REQUIRES: intel_feature_isa_avx_dotprod_int8
// RUN: %clang -target i386-unknown-unknown -march=atom -mavxdotprodint8 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXDOTPRODINT8 %s

// AVXDOTPRODINT8: #define __AVX2__ 1
// AVXDOTPRODINT8: #define __AVXDOTPRODINT8__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mno-avxdotprodint8 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=NOAVXDOTPRODINT8 %s

// NOAVXDOTPRODINT8-NOT: #define __AVXDOTPRODINT8__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavxdotprodint8 -mno-avx2 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXDOTPRODINT8NOAVX512BF16 %s

// AVXDOTPRODINT8NOAVX512BF16-NOT: #define __AVX2__ 1
// AVXDOTPRODINT8NOAVX512BF16-NOT: #define __AVXDOTPRODINT8__ 1

#endif // INTEL_FEATURE_ISA_AVX_DOTPROD_INT8
