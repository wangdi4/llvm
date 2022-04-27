#if INTEL_FEATURE_ISA_AVX512_VNNI_INT8
// REQUIRES: intel_feature_isa_avx512_vnni_int8
// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512vnniint8 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512VNNIINT8 %s

// AVX512VNNIINT8: #define __AVX512F__ 1
// AVX512VNNIINT8: #define __AVX512VNNIINT8__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mno-avx512vnniint8 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=NOAVX512VNNIINT8 %s

// NOAVX512VNNIINT8-NOT: #define __AVX512VNNIINT8__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512vnniint8 -mno-avx512f -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512VNNIINT8NOAVX512F %s

// AVX512VNNIINT8NOAVX512F-NOT: #define __AVX512VNNIINT8__ 1
// AVX512VNNIINT8NOAVX512F-NOT: #define __AVX512F__ 1


#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT8
