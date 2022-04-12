#if INTEL_FEATURE_ISA_AVX_VNNI_INT8
// REQUIRES: intel_feature_isa_avx_vnni_int8
// RUN: %clang -target i386-unknown-unknown -march=atom -mavxvnniint8 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXVNNIINT8 %s

// AVXVNNIINT8: #define __AVX2__ 1
// AVXVNNIINT8: #define __AVXVNNIINT8__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mno-avxvnniint8 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=NOAVXVNNIINT8 %s

// NOAVXVNNIINT8-NOT: #define __AVXVNNIINT8__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavxvnniint8 -mno-avx2 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXVNNIINT8NOAVX512BF16 %s

// AVXVNNIINT8NOAVX512BF16-NOT: #define __AVX2__ 1
// AVXVNNIINT8NOAVX512BF16-NOT: #define __AVXVNNIINT8__ 1

#endif // INTEL_FEATURE_ISA_AVX_VNNI_INT8
