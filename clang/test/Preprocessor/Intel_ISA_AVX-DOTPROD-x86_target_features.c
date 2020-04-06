#if INTEL_FEATURE_ISA_AVX_DOTPROD
// REQUIRES: intel_feature_isa_avx_dotprod
// RUN: %clang -target i386-unknown-unknown -march=atom -mavxdotprod -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXDOTPROD %s

// AVXDOTPROD: #define __AVX2__ 1
// AVXDOTPROD: #define __AVXDOTPROD__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mno-avxdotprod -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=NOAVXDOTPROD %s

// NOAVXDOTPROD-NOT: #define __AVXDOTPROD__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavxdotprod -mno-avx2 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXDOTPRODNOAVX512BF16 %s

// AVXDOTPRODNOAVX512BF16-NOT: #define __AVX2__ 1
// AVXDOTPRODNOAVX512BF16-NOT: #define __AVXDOTPROD__ 1

#endif // INTEL_FEATURE_ISA_AVX_DOTPROD
