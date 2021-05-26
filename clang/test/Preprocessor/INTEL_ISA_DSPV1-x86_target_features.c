#if INTEL_FEATURE_ISA_DSPV1
// REQUIRES: intel_feature_isa_dspv1

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mdspv1 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=DSP %s
// DSP: #define __DSPV1__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-dspv1 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-DSP %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mdspv1 -mno-dspv1 \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-DSP %s
// NO-DSP-NOT: #define __DSPV1__ 1

#endif // INTEL_FEATURE_ISA_DSPV1
