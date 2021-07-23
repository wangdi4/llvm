#if INTEL_FEATURE_ISA_AVX_NE_CONVERT
// REQUIRES: intel_feature_isa_avx_ne_convert

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavxneconvert -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVXNECONVERT %s
// AVXNECONVERT: #define __AVXNECONVERT__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-avxneconvert -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVXNECONVERT %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavxneconvert -mno-avx2 \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVXNECONVERT %s
// NO-AVXNECONVERT-NOT: #define __AVXNECONVERT__ 1

#endif // INTEL_FEATURE_ISA_AVX_NE_CONVERT