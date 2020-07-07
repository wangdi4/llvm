#if INTEL_FEATURE_ISA_AVX512_DOTPROD_INT8
// REQUIRES: intel_feature_isa_avx512_dotprod_int8
// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512dotprodint8 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512DOTPRODINT8 %s

// AVX512DOTPRODINT8: #define __AVX512DOTPRODINT8__ 1
// AVX512DOTPRODINT8: #define __AVX512F__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mno-avx512dotprodint8 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=NOAVX512DOTPRODINT8 %s

// NOAVX512DOTPRODINT8-NOT: #define __AVX512DOTPRODINT8__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512dotprodint8 -mno-avx512f -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512DOTPRODINT8NOAVX512F %s

// AVX512DOTPRODINT8NOAVX512F-NOT: #define __AVX512DOTPRODINT8__ 1
// AVX512DOTPRODINT8NOAVX512F-NOT: #define __AVX512F__ 1


#endif // INTEL_FEATURE_ISA_AVX512_DOTPROD_INT8
