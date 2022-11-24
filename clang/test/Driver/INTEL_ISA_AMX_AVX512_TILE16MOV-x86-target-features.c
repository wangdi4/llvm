#if INTEL_FEATURE_ISA_AMX_AVX512_TILE16MOV
// REQUIRES: intel_feature_isa_amx_avx512_tile16mov

// RUN: %clang -target x86_64-unknown-linux-gnu -mamx-avx512-tile16mov %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-AVX512-TILE16MOV %s
// RUN: %clang -target x86_64-unknown-linux-gnu -mno-amx-avx512-tile16mov %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-AVX512-TILE16MOV %s
// AMX-AVX512-TILE16MOV: "-target-feature" "+amx-avx512-tile16mov"
// NO-AMX-AVX512-TILE16MOV: "-target-feature" "-amx-avx512-tile16mov"
#endif // INTEL_FEATURE_ISA_AMX_AVX512_TILE16MOV