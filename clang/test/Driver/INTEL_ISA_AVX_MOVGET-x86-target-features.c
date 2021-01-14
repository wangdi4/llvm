#if INTEL_FEATURE_ISA_AVX_MOVGET
// REQUIRES: intel_feature_isa_avx_movget

// RUN: %clang -target x86_64-unknown-linux-gnu -mavxmovget %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX-MOVGET %s
// RUN: %clang -target x86_64-unknown-linux-gnu -mno-avxmovget %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX-MOVGET %s
// AVX-MOVGET: "-target-feature" "+avxmovget"
// NO-AVX-MOVGET: "-target-feature" "-avxmovget"
#endif // INTEL_FEATURE_ISA_AVX_MOVGET
