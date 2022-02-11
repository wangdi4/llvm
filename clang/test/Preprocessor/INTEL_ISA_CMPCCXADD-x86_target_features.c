#if INTEL_FEATURE_ISA_CMPCCXADD
// REQUIRES: intel_feature_isa_cmpccxadd

// RUN: %clang -target x86_64-unknown-linux-gnu -march=atom -mcmpccxadd -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=CMPCCXADD %s
// CMPCCXADD: #define __CMPCCXADD__ 1
// RUN: %clang -target x86_64-unknown-linux-gnu -march=atom -mno-cmpccxadd -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-CMPCCXADD %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=atom -mcmpccxadd -mno-avx2 \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-CMPCCXADD %s
// NO-CMPCCXADD-NOT: #define __CMPCCXADD__ 1

#endif // INTEL_FEATURE_ISA_CMPCCXADD
