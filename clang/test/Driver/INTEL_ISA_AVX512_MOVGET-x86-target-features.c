#if INTEL_FEATURE_ISA_AVX512_MOVGET
// REQUIRES: intel_feature_isa_avx512_movget

// RUN: %clang -target x86_64-unknown-linux-gnu -mavx512movget %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512-MOVGET %s
// RUN: %clang -target x86_64-unknown-linux-gnu -mno-avx512movget %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512-MOVGET %s
// AVX512-MOVGET: "-target-feature" "+avx512movget"
// NO-AVX512-MOVGET: "-target-feature" "-avx512movget"
#endif // INTEL_FEATURE_ISA_AVX512_MOVGET
