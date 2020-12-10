#if INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
// REQUIRES: intel_feature_isa_amx_memadvise_evex

// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-memadvise-evex -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-MEMADVISE-EVEX %s
// AMX-MEMADVISE-EVEX: #define __AMXMEMADVISEEVEX__ 1
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mno-amx-memadvise-evex -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-MEMADVISE-EVEX %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-memadvise-evex -mno-amx-tile \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-MEMADVISE-EVEX %s
// NO-AMX-MEMADVISE-EVEX-NOT: #define __AMXMEMADVISEEVEX__ 1

#endif // INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX