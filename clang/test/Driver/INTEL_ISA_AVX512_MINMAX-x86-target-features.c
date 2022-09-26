#if INTEL_FEATURE_ISA_AVX512_MINMAX
// REQUIRES: intel_feature_isa_avx512_minmax

// RUN: %clang -target i686-unknown-linux-gnu -mavx512minmax %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512MINMAX %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-avx512minmax %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512MINMAX %s
// AVX512MINMAX: "-target-feature" "+avx512minmax"
// NO-AVX512MINMAX: "-target-feature" "-avx512minmax"
#endif // INTEL_FEATURE_ISA_AVX512_MINMAX