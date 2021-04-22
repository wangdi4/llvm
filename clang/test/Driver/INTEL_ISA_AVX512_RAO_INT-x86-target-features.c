#if INTEL_FEATURE_ISA_AVX512_RAO_INT
// REQUIRES: intel_feature_isa_avx512_rao_int

// RUN: %clang -target i686-unknown-linux-gnu -mavx512raoint %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512RAOINT %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-avx512raoint %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512RAOINT %s
// AVX512RAOINT: "-target-feature" "+avx512raoint"
// NO-AVX512RAOINT: "-target-feature" "-avx512raoint"
#endif // INTEL_FEATURE_ISA_AVX512_RAO_INT