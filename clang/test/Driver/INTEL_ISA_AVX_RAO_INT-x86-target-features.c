#if INTEL_FEATURE_ISA_AVX_RAO_INT
// REQUIRES: intel_feature_isa_avx_rao_int

// RUN: %clang -target i686-unknown-linux-gnu -mavxraoint %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVXRAOINT %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-avxraoint %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVXRAOINT %s
// AVXRAOINT: "-target-feature" "+avxraoint"
// NO-AVXRAOINT: "-target-feature" "-avxraoint"
#endif // INTEL_FEATURE_ISA_AVX_RAO_INT