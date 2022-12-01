#if INTEL_FEATURE_ISA_DSPV1
// REQUIRES: intel_feature_isa_dspv1

// RUN: %clang -target i686-unknown-linux-gnu -mdspv1 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=DSP %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-dspv1 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-DSP %s
// DSP: "-target-feature" "+dspv1"
// NO-DSP: "-target-feature" "-dspv1"

// Feature dspv1 can only enabled on 32bit target.
// RUN: not %clang -target x86_64-unknown-linux-gnu -mdspv1 %s \
// RUN: 2>&1 | FileCheck %s -check-prefix=ERROR
// ERROR: error: option '-mdspv1' cannot be specified on this target
#endif // INTEL_FEATURE_ISA_DSPV1
