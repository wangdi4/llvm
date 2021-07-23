#if INTEL_FEATURE_ISA_AVX512_VNNI_INT16
// REQUIRES: intel_feature_isa_avx512_vnni_int16

// RUN: %clang -target i686-unknown-linux-gnu -mavx512vnniint16 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVX512VNNIINT16 %s
// AVX512VNNIINT16: #define __AVX512VNNIINT16__ 1
// RUN: %clang -target i686-unknown-linux-gnu -mno-avx512vnniint16 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512VNNIINT16 %s
// RUN: %clang -target i686-unknown-linux-gnu -mavx512vnniint16 -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVX512VNNIINT16 %s
// NO-AVX512VNNIINT16-NOT: #define __AVX512VNNIINT16__ 1

#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT16
