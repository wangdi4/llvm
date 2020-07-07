#if INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
// REQUIRES: intel_feature_isa_avx_dotprod_phps
// RUN: %clang -target i386-unknown-unknown -march=atom -mavxdotprodphps -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXDOTPRODPHPS %s

// AVXDOTPRODPHPS: #define __AVX2__ 1
// AVXDOTPRODPHPS: #define __AVXDOTPRODPHPS__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mno-avxdotprodphps -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=NOAVXDOTPRODPHPS %s

// NOAVXDOTPRODPHPS-NOT: #define __AVXDOTPRODPHPS__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavxdotprodphps -mno-avx2 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXDOTPRODPHPSNOAVX512BF16 %s

// AVXDOTPRODPHPSNOAVX512BF16-NOT: #define __AVX2__ 1
// AVXDOTPRODPHPSNOAVX512BF16-NOT: #define __AVXDOTPRODPHPS__ 1

#endif // INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
