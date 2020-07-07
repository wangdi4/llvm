#if INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
// REQUIRES: intel_feature_isa_avx_dotprod_phps

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavxdotprodphps %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX-DOTPRODPHPS %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avxdotprodphps \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX-DOTPRODPHPS %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avxdotprodphps \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX-DOTPRODPHPS %s
// AVX-DOTPRODPHPS: "-target-feature" "+avxdotprodphps"
// NO-AVX-DOTPRODPHPS: "-target-feature" "-avxdotprodphps"

#endif // INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
