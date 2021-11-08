#if INTEL_FEATURE_ISA_VPINSR_VPEXTR
// REQUIRES: intel_feature_isa_vpinsr_vpextr

// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mvpinsr-vpextr -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=VPINSR-VPEXTR %s
// VPINSR-VPEXTR: #define __VPINSRVPEXTR__ 1
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mno-amx-v3 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-VPINSR-VPEXTR %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mvpinsr-vpextr -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-VPINSR-VPEXTR %s
// NO-VPINSR-VPEXTR-NOT: #define __VPINSRVPEXTR__ 1

#endif // INTEL_FEATURE_ISA_VPINSR_VPEXTR
