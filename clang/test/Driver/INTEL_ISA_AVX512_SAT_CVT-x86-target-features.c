#if INTEL_FEATURE_ISA_AVX512_SAT_CVT
// REQUIRES: intel_feature_isa_avx512_sat_cvt

// RUN: %clang -target i686-unknown-linux-gnu -mavx512satcvt %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512SATCVT %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-avx512satcvt %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512SATCVT %s
// AVX512SATCVT: "-target-feature" "+avx512satcvt"
// NO-AVX512SATCVT: "-target-feature" "-avx512satcvt"
#endif // INTEL_FEATURE_ISA_AVX512_SAT_CVT