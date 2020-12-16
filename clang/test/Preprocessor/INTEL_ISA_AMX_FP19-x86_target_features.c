#if INTEL_FEATURE_ISA_AMX_FP19
// REQUIRES: intel_feature_isa_amx_fp19

// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-fp19 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-FP19 %s
// AMX-FP19: #define __AMXFP19__ 1
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mno-amx-fp19 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-FP19 %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-fp19 -mno-amx-tile \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-FP19 %s
// NO-AMX-FP19-NOT: #define __AMXFP19__ 1

#endif // INTEL_FEATURE_ISA_AMX_FP19