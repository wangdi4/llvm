#if INTEL_FEATURE_ISA_AVX512_DOTPROD_PHPS
// REQUIRES: intel_feature_isa_avx512_dotprod_phps
// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512dotprodphps -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512DOTPRODPHPS %s

// AVX512DOTPRODPHPS: #define __AVX512BW__ 1
// AVX512DOTPRODPHPS: #define __AVX512DOTPRODPHPS__ 1
// AVX512DOTPRODPHPS: #define __AVX512DQ__ 1
// AVX512DOTPRODPHPS: #define __AVX512FP16__ 1
// AVX512DOTPRODPHPS: #define __AVX512F__ 1
// AVX512DOTPRODPHPS: #define __AVX512VL__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mno-avx512dotprodphps -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=NOAVX512DOTPRODPHPS %s

// NOAVX512DOTPRODPHPS-NOT: #define __AVX512DOTPRODPHPS__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512dotprodphps -mno-avx512fp16 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512DOTPRODPHPSNOAVX512FP16 %s

// AVX512DOTPRODPHPSNOAVX512FP16-NOT: #define __AVX512DOTPRODPHPS__ 1
// AVX512DOTPRODPHPSNOAVX512FP16-NOT: #define __AVX512FP16__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512dotprodphps -mno-avx512vl -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512DOTPRODPHPSNOAVX512VL %s

// AVX512DOTPRODPHPSNOAVX512VL-NOT: #define __AVX512DOTPRODPHPS__ 1
// AVX512DOTPRODPHPSNOAVX512VL-NOT: #define __AVX512FP16__ 1
// AVX512DOTPRODPHPSNOAVX512VL-NOT: #define __AVX512VL__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512dotprodphps -mno-avx512bw -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512DOTPRODPHPSNOAVX512BW %s

// AVX512DOTPRODPHPSNOAVX512BW-NOT: #define __AVX512BW__ 1
// AVX512DOTPRODPHPSNOAVX512BW-NOT: #define __AVX512DOTPRODPHPS__ 1
// AVX512DOTPRODPHPSNOAVX512BW-NOT: #define __AVX512FP16__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512dotprodphps -mno-avx512f -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512DOTPRODPHPSNOAVX512F %s

// AVX512DOTPRODPHPSNOAVX512F-NOT: #define __AVX512BW__ 1
// AVX512DOTPRODPHPSNOAVX512F-NOT: #define __AVX512DOTPRODPHPS__ 1
// AVX512DOTPRODPHPSNOAVX512F-NOT: #define __AVX512DQ__ 1
// AVX512DOTPRODPHPSNOAVX512F-NOT: #define __AVX512FP16__ 1
// AVX512DOTPRODPHPSNOAVX512F-NOT: #define __AVX512F__ 1
// AVX512DOTPRODPHPSNOAVX512F-NOT: #define __AVX512VL__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512dotprodphps -mno-avx512dq -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512DOTPRODPHPSNOAVX512DQ %s

// AVX512DOTPRODPHPSNOAVX512DQ-NOT: #define __AVX512DOTPRODPHPS__ 1
// AVX512DOTPRODPHPSNOAVX512DQ-NOT: #define __AVX512DQ__ 1
// AVX512DOTPRODPHPSNOAVX512DQ-NOT: #define __AVX512FP16__ 1

#endif // INTEL_FEATURE_ISA_AVX512_DOTPROD_PHPS
