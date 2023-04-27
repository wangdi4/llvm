#if INTEL_FEATURE_ISA_UMSR
// REQUIRES: intel_feature_isa_umsr

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mumsr -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=UMSR %s
// UMSR: #define __UMSR__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-umsr -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-UMSR %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mumsr -mno-umsr \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-UMSR %s
// NO-UMSR-NOT: #define __UMSR__ 1

#endif // INTEL_FEATURE_ISA_UMSR
