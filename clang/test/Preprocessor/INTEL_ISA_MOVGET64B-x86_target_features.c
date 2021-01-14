#if INTEL_FEATURE_ISA_MOVGET64B
// REQUIRES: intel_feature_isa_movget64b

// RUN: %clang -target i686-unknown-linux-gnu -march=x86-64 -mmovget64b -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=MOVGET64B %s
// MOVGET64B: #define __MOVGET64B__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=x86-64 -mno-movget64b -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-MOVGET64B %s
// RUN: %clang -target i686-unknown-linux-gnu -march=x86-64 -mmovget64b -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-MOVGET64B %s
// NO-MOVGET64B-NOT: #define __MOVGET64B__ 1

#endif // INTEL_FEATURE_ISA_MOVGET64B
