#if INTEL_FEATURE_ISA_AVX_RAO_FP
// REQUIRES: intel_feature_isa_avx_rao_fp

// RUN: %clang -target i686-unknown-linux-gnu -mavxraofp %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVXRAOFP %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-avxraofp %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVXRAOFP %s
// AVXRAOFP: "-target-feature" "+avxraofp"
// NO-AVXRAOFP: "-target-feature" "-avxraofp"
#endif // INTEL_FEATURE_ISA_AVX_RAO_FP