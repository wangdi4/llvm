#if INTEL_FEATURE_ISA_DSPV1
// REQUIRES: intel_feature_isa_dspv1

// RUN: %clang -target i686-unknown-linux-gnu -mdspv1 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=DSP %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-dspv1 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-DSP %s
// DSP: "-target-feature" "+dspv1"
// NO-DSP: "-target-feature" "-dspv1"
#endif // INTEL_FEATURE_ISA_DSPV1
