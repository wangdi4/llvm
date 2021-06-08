#if INTEL_FEATURE_ISA_SM3
// REQUIRES: intel_feature_isa_sm3

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -msm3 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=SM3 %s
// SM3: #define __SM3__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-sm3 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-SM3 %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -msm3 -mno-avx \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-SM3 %s
// NO-SM3-NOT: #define __SM3__ 1

#endif // INTEL_FEATURE_ISA_SM3
