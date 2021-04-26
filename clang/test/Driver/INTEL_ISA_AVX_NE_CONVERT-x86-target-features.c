#if INTEL_FEATURE_ISA_AVX_NE_CONVERT
// REQUIRES: intel_feature_isa_avx_ne_convert

// RUN: %clang -target i686-unknown-linux-gnu -mavxneconvert %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVXNECONVERT %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-avxneconvert %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVXNECONVERT %s
// AVXNECONVERT: "-target-feature" "+avxneconvert"
// NO-AVXNECONVERT: "-target-feature" "-avxneconvert"
#endif // INTEL_FEATURE_ISA_AVX_NE_CONVERT