#if INTEL_FEATURE_ISA_AVX512_VNNI_FP16
// REQUIRES: intel_feature_isa_avx512_vnni_fp16
// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512vnnifp16 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512VNNIFP16 %s

// AVX512VNNIFP16: #define __AVX512BW__ 1
// AVX512VNNIFP16: #define __AVX512DQ__ 1
// AVX512VNNIFP16: #define __AVX512FP16__ 1
// AVX512VNNIFP16: #define __AVX512F__ 1
// AVX512VNNIFP16: #define __AVX512VL__ 1
// AVX512VNNIFP16: #define __AVX512VNNIFP16__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mno-avx512vnnifp16 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=NOAVX512VNNIFP16 %s

// NOAVX512VNNIFP16-NOT: #define __AVX512VNNIFP16__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512vnnifp16 -mno-avx512fp16 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512VNNIFP16NOAVX512FP16 %s

// AVX512VNNIFP16NOAVX512FP16-NOT: #define __AVX512VNNIFP16__ 1
// AVX512VNNIFP16NOAVX512FP16-NOT: #define __AVX512FP16__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512vnnifp16 -mno-avx512vl -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512VNNIFP16NOAVX512VL %s

// AVX512VNNIFP16NOAVX512VL-NOT: #define __AVX512VNNIFP16__ 1
// AVX512VNNIFP16NOAVX512VL-NOT: #define __AVX512FP16__ 1
// AVX512VNNIFP16NOAVX512VL-NOT: #define __AVX512VL__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512vnnifp16 -mno-avx512bw -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512VNNIFP16NOAVX512BW %s

// AVX512VNNIFP16NOAVX512BW-NOT: #define __AVX512BW__ 1
// AVX512VNNIFP16NOAVX512BW-NOT: #define __AVX512VNNIFP16__ 1
// AVX512VNNIFP16NOAVX512BW-NOT: #define __AVX512FP16__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512vnnifp16 -mno-avx512f -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512VNNIFP16NOAVX512F %s

// AVX512VNNIFP16NOAVX512F-NOT: #define __AVX512BW__ 1
// AVX512VNNIFP16NOAVX512F-NOT: #define __AVX512VNNIFP16__ 1
// AVX512VNNIFP16NOAVX512F-NOT: #define __AVX512DQ__ 1
// AVX512VNNIFP16NOAVX512F-NOT: #define __AVX512FP16__ 1
// AVX512VNNIFP16NOAVX512F-NOT: #define __AVX512F__ 1
// AVX512VNNIFP16NOAVX512F-NOT: #define __AVX512VL__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512vnnifp16 -mno-avx512dq -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVX512VNNIFP16NOAVX512DQ %s

// AVX512VNNIFP16NOAVX512DQ-NOT: #define __AVX512VNNIFP16__ 1
// AVX512VNNIFP16NOAVX512DQ-NOT: #define __AVX512DQ__ 1
// AVX512VNNIFP16NOAVX512DQ-NOT: #define __AVX512FP16__ 1

#endif // INTEL_FEATURE_ISA_AVX512_VNNI_FP16
