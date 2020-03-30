#if INTEL_FEATURE_ISA_AVX_DOTPROD
// REQUIRES: intel_feature_isa_avx_dotprod

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavxdotprod %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX-DOTPROD %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avxdotprod \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX-DOTPROD %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avxdotprod \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX-DOTPROD %s
// AVX-DOTPROD: "-target-feature" "+avxdotprod"
// NO-AVX-DOTPROD: "-target-feature" "-avxdotprod"

#endif // INTEL_FEATURE_ISA_AVX_DOTPROD
