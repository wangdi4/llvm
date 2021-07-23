#if INTEL_FEATURE_ISA_AVX512_NE_CONVERT
// REQUIRES: intel_feature_isa_avx512_ne_convert

// RUN: %clang -target i686-unknown-linux-gnu -mavx512neconvert %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512NECONVERT %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-avx512neconvert %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512NECONVERT %s
// AVX512NECONVERT: "-target-feature" "+avx512neconvert"
// NO-AVX512NECONVERT: "-target-feature" "-avx512neconvert"
#endif // INTEL_FEATURE_ISA_AVX512_NE_CONVERT