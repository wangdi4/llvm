#if INTEL_FEATURE_ISA_AVX_VNNI_INT16
// REQUIRES: intel_feature_isa_avx_vnni_int16

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavxvnniint16 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AVXVNNIINT16 %s
// AVXVNNIINT16: #define __AVXVNNIINT16__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-avxvnniint16 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AVXVNNIINT16 %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mavxvnniint16 -mno-avx2 \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AVXVNNIINT16 %s
// NO-AVXVNNIINT16-NOT: #define __AVXVNNIINT16__ 1

#endif // INTEL_FEATURE_ISA_AVX_VNNI_INT16