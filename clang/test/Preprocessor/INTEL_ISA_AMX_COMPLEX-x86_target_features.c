#if INTEL_FEATURE_ISA_AMX_COMPLEX
// REQUIRES: intel_feature_isa_amx_complex

// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-complex -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-COMPLEX %s
// AMX-COMPLEX: #define __AMXCOMPLEX__ 1
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mno-amx-complex -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-COMPLEX %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-complex -mno-amx-tile \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-COMPLEX %s
// NO-AMX-COMPLEX-NOT: #define __AMXCOMPLEX__ 1

#endif // INTEL_FEATURE_ISA_AMX_COMPLEX
