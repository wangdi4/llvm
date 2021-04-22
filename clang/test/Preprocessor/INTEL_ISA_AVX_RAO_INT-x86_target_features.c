#if INTEL_FEATURE_ISA_AVX_RAO_INT
// REQUIRES: intel_feature_isa_avx_rao_int

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavxraoint -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVXRAOINT %s
// AVXRAOINT: #define __AVXRAOINT__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-avxraoint -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVXRAOINT %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavxraoint -mno-avx2 \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVXRAOINT %s
// NO-AVXRAOINT-NOT: #define __AVXRAOINT__ 1

#endif // INTEL_FEATURE_ISA_AVX_RAO_INT