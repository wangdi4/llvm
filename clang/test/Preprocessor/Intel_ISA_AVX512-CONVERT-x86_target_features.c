#if INTEL_FEATURE_ISA_AVX512_CONVERT
// REQUIRES: intel_feature_isa_avx512_convert
// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512convert -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512CONVERT %s

// AVX512CONVERT: #define __AVX512BW__ 1
// AVX512CONVERT: #define __AVX512CONVERT__ 1
// AVX512CONVERT: #define __AVX512DQ__ 1
// AVX512CONVERT: #define __AVX512FP16__ 1
// AVX512CONVERT: #define __AVX512F__ 1
// AVX512CONVERT: #define __AVX512VL__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mno-avx512convert -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=NOAVX512CONVERT %s

// NOAVX512CONVERT-NOT: #define __AVX512CONVERT__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512convert -mno-avx512fp16 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512CONVERTNOAVX512FP16 %s

// AVX512CONVERTNOAVX512FP16-NOT: #define __AVX512CONVERT__ 1
// AVX512CONVERTNOAVX512FP16-NOT: #define __AVX512FP16__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512convert -mno-avx512bf16 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512CONVERTNOAVX512BF16 %s

// AVX512CONVERTNOAVX512BF16-NOT: #define __AVX512CONVERT__ 1
// AVX512CONVERTNOAVX512BF16-NOT: #define __AVX512BF16__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512convert -mno-avx512vl -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512CONVERTNOAVX512VL %s

// AVX512CONVERTNOAVX512VL-NOT: #define __AVX512CONVERT__ 1
// AVX512CONVERTNOAVX512VL-NOT: #define __AVX512FP16__ 1
// AVX512CONVERTNOAVX512VL-NOT: #define __AVX512VL__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512convert -mno-avx512bw -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512CONVERTNOAVX512BW %s

// AVX512CONVERTNOAVX512BW-NOT: #define __AVX512BW__ 1
// AVX512CONVERTNOAVX512BW-NOT: #define __AVX512CONVERT__ 1
// AVX512CONVERTNOAVX512BW-NOT: #define __AVX512FP16__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512convert -mno-avx512f -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512CONVERTNOAVX512F %s

// AVX512CONVERTNOAVX512F-NOT: #define __AVX512BW__ 1
// AVX512CONVERTNOAVX512F-NOT: #define __AVX512CONVERT__ 1
// AVX512CONVERTNOAVX512F-NOT: #define __AVX512DQ__ 1
// AVX512CONVERTNOAVX512F-NOT: #define __AVX512FP16__ 1
// AVX512CONVERTNOAVX512F-NOT: #define __AVX512F__ 1
// AVX512CONVERTNOAVX512F-NOT: #define __AVX512VL__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512convert -mno-avx512dq -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512CONVERTNOAVX512DQ %s

// AVX512CONVERTNOAVX512DQ-NOT: #define __AVX512CONVERT__ 1
// AVX512CONVERTNOAVX512DQ-NOT: #define __AVX512DQ__ 1
// AVX512CONVERTNOAVX512DQ-NOT: #define __AVX512FP16__ 1

#endif // INTEL_FEATURE_ISA_AVX512_CONVERT
