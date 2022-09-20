#if INTEL_FEATURE_ISA_AVX512_REDUCTION
// REQUIRES: intel_feature_isa_avx512_reduction

// RUN: %clang -target i686-unknown-linux-gnu -mavx512reduction %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512REDUCTION %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-avx512reduction %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512REDUCTION %s
// AVX512REDUCTION: "-target-feature" "+avx512reduction"
// NO-AVX512REDUCTION: "-target-feature" "-avx512reduction"
#endif // INTEL_FEATURE_ISA_AVX512_REDUCTION