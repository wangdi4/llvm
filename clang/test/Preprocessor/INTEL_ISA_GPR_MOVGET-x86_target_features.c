#if INTEL_FEATURE_ISA_GPR_MOVGET
// REQUIRES: intel_feature_isa_gpr_movget

// RUN: %clang -target i686-unknown-linux-gnu -march=x86-64 -mgprmovget -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=GPRMOVGET %s
// GPRMOVGET: #define __GPRMOVGET__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=x86-64 -mno-gprmovget -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-GPRMOVGET %s
// NO-GPRMOVGET-NOT: #define __GPRMOVGET__ 1

#endif // INTEL_FEATURE_ISA_GPR_MOVGET
