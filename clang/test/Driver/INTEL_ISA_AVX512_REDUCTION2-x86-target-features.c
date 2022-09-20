#if INTEL_FEATURE_ISA_AVX512_REDUCTION2
// REQUIRES: intel_feature_isa_avx512_reduction2

// RUN: %clang -target i686-unknown-linux-gnu -mavx512reduction2 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512REDUCTION2 %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-avx512reduction2 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512REDUCTION2 %s
// AVX512REDUCTION2: "-target-feature" "+avx512reduction2"
// NO-AVX512REDUCTION2: "-target-feature" "-avx512reduction2"
#endif // INTEL_FEATURE_ISA_AVX512_REDUCTION2