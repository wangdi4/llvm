#if INTEL_FEATURE_ISA_AMX_AVX512_TILE16MOV
// REQUIRES: intel_feature_isa_amx_avx512_tile16mov

// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-avx512-tile16mov -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-AVX512-TILE16MOV %s
// AMX-AVX512-TILE16MOV: #define __AMXAVX512TILE16MOV__ 1
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mno-amx-avx512-tile16mov -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-AVX512-TILE16MOV %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-avx512-tile16mov -mno-amx-tile \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-AVX512-TILE16MOV %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-avx512-tile16mov -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-AVX512-TILE16MOV %s
// NO-AMX-AVX512-TILE16MOV-NOT: #define __AMXAVX512TILE16MOV__ 1

#endif // INTEL_FEATURE_ISA_AMX_AVX512_TILE16MOV
