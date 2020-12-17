#if INTEL_FEATURE_ISA_AMX_COMPLEX_EVEX
// REQUIRES: intel_feature_isa_amx_complex_evex

// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-complex-evex -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-COMPLEX-EVEX %s
// AMX-COMPLEX-EVEX: #define __AMXCOMPLEXEVEX__ 1
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mno-amx-complex-evex -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-COMPLEX-EVEX %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-complex-evex -mno-amx-tile \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-COMPLEX-EVEX %s
// NO-AMX-COMPLEX-EVEX-NOT: #define __AMXCOMPLEXEVEX__ 1

#endif // INTEL_FEATURE_ISA_AMX_COMPLEX_EVEX
