#if INTEL_FEATURE_ISA_AVX_RAO_FP
// REQUIRES: intel_feature_isa_avx_rao_fp

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavxraofp -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVXRAOFP %s
// AVXRAOFP: #define __AVXRAOFP__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-avxraofp -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVXRAOFP %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavxraofp -mno-avx2 \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVXRAOFP %s
// NO-AVXRAOFP-NOT: #define __AVXRAOFP__ 1

#endif // INTEL_FEATURE_ISA_AVX_RAO_FP