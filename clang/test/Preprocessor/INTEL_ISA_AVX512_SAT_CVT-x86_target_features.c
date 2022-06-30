#if INTEL_FEATURE_ISA_AVX512_SAT_CVT
// REQUIRES: intel_feature_isa_avx512_sat_cvt

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512satcvt -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVX512SATCVT %s
// AVX512SATCVT: #define __AVX512SATCVT__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-avx512satcvt -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512SATCVT %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512satcvt -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512SATCVT %s
// NO-AVX512SATCVT-NOT: #define __AVX512SATCVT__ 1

#endif // INTEL_FEATURE_ISA_AVX512_SAT_CVT