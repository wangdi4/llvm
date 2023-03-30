#if INTEL_FEATURE_ISA_AVX512_MOVZXC
// REQUIRES: intel_feature_isa_avx512_movzxc

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512movzxc -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVX512MOVZXC %s
// AVX512MOVZXC: #define __AVX512MOVZXC__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-avx512movzxc -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512MOVZXC %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavx512movzxc -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512MOVZXC %s
// NO-AVX512MOVZXC-NOT: #define __AVX512MOVZXC__ 1

#endif // INTEL_FEATURE_ISA_AVX512_MOVZXC
