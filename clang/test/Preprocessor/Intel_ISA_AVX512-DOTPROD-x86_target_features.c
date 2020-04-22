#if INTEL_FEATURE_ISA_AVX512_DOTPROD
// REQUIRES: intel_feature_isa_avx512_dotprod
// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512dotprod -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512DOTPROD %s

// AVX512DOTPROD: #define __AVX512BW__ 1
// AVX512DOTPROD: #define __AVX512DOTPROD__ 1
// AVX512DOTPROD: #define __AVX512DQ__ 1
// AVX512DOTPROD: #define __AVX512FP16__ 1
// AVX512DOTPROD: #define __AVX512F__ 1
// AVX512DOTPROD: #define __AVX512VL__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mno-avx512dotprod -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=NOAVX512DOTPROD %s

// NOAVX512DOTPROD-NOT: #define __AVX512DOTPROD__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512dotprod -mno-avx512fp16 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512DOTPRODNOAVX512FP16 %s

// AVX512DOTPRODNOAVX512FP16-NOT: #define __AVX512DOTPROD__ 1
// AVX512DOTPRODNOAVX512FP16-NOT: #define __AVX512FP16__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512dotprod -mno-avx512vl -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512DOTPRODNOAVX512VL %s

// AVX512DOTPRODNOAVX512VL-NOT: #define __AVX512DOTPROD__ 1
// AVX512DOTPRODNOAVX512VL-NOT: #define __AVX512FP16__ 1
// AVX512DOTPRODNOAVX512VL-NOT: #define __AVX512VL__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512dotprod -mno-avx512bw -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512DOTPRODNOAVX512BW %s

// AVX512DOTPRODNOAVX512BW-NOT: #define __AVX512BW__ 1
// AVX512DOTPRODNOAVX512BW-NOT: #define __AVX512DOTPROD__ 1
// AVX512DOTPRODNOAVX512BW-NOT: #define __AVX512FP16__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512dotprod -mno-avx512f -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512DOTPRODNOAVX512F %s

// AVX512DOTPRODNOAVX512F-NOT: #define __AVX512BW__ 1
// AVX512DOTPRODNOAVX512F-NOT: #define __AVX512DOTPROD__ 1
// AVX512DOTPRODNOAVX512F-NOT: #define __AVX512DQ__ 1
// AVX512DOTPRODNOAVX512F-NOT: #define __AVX512FP16__ 1
// AVX512DOTPRODNOAVX512F-NOT: #define __AVX512F__ 1
// AVX512DOTPRODNOAVX512F-NOT: #define __AVX512VL__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512dotprod -mno-avx512dq -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512DOTPRODNOAVX512DQ %s

// AVX512DOTPRODNOAVX512DQ-NOT: #define __AVX512DOTPROD__ 1
// AVX512DOTPRODNOAVX512DQ-NOT: #define __AVX512DQ__ 1
// AVX512DOTPRODNOAVX512DQ-NOT: #define __AVX512FP16__ 1

#endif // INTEL_FEATURE_ISA_AVX512_DOTPROD
