#if INTEL_FEATURE_ISA_FP16
// REQUIRES: intel_feature_isa_fp16
// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512fp16 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512FP16 %s

// AVX512FP16: #define __AVX512BW__ 1
// AVX512FP16: #define __AVX512DQ__ 1
// AVX512FP16: #define __AVX512FP16__ 1
// AVX512FP16: #define __AVX512VL__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512fp16 -mno-avx512vl -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512FP16NOAVX512VL %s

// AVX512FP16NOAVX512VL-NOT: #define __AVX512FP16__ 1
// AVX512FP16NOAVX512VL-NOT: #define __AVX512VL__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512fp16 -mno-avx512bw -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512FP16NOAVX512BW %s

// AVX512FP16NOAVX512BW-NOT: #define __AVX512BW__ 1
// AVX512FP16NOAVX512BW-NOT: #define __AVX512FP16__ 1
//
// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512fp16 -mno-avx512dq -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512FP16NOAVX512DQ %s

// AVX512FP16NOAVX512DQ-NOT: #define __AVX512DQ__ 1
// AVX512FP16NOAVX512DQ-NOT: #define __AVX512FP16__ 1
#endif // INTEL_FEATURE_ISA_FP16
