#if INTEL_FEATURE_ISA_AMX_V3
// REQUIRES: intel_feature_isa_amx_v3

// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-v3 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-V3 %s
// AMX-V3: #define __AMXV3__ 1
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mno-amx-v3 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-V3 %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-v3 -mno-amx-tile \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-V3 %s
// NO-AMX-V3-NOT: #define __AMXV3__ 1

#endif // INTEL_FEATURE_ISA_AMX_V3