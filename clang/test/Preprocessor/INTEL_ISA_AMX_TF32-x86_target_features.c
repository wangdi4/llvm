#if INTEL_FEATURE_ISA_AMX_TF32
// REQUIRES: intel_feature_isa_amx_tf32

// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-tf32 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-TF32 %s
// AMX-TF32: #define __AMXTF32__ 1
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mno-amx-tf32 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-TF32 %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-tf32 -mno-amx-tile \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-TF32 %s
// NO-AMX-TF32-NOT: #define __AMXTF32__ 1

#endif // INTEL_FEATURE_ISA_AMX_TF32