#if INTEL_FEATURE_ISA_SM4
// REQUIRES: intel_feature_isa_sm4

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -msm4 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=SM4 %s
// SM4: #define __SM4__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-sm4 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-SM4 %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -msm4 -mno-avx \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-SM4 %s
// NO-SM4-NOT: #define __SM4__ 1

#endif // INTEL_FEATURE_ISA_SM4
