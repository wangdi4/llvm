#if INTEL_FEATURE_ISA_AMX_AVX512
// REQUIRES: intel_feature_isa_amx_avx512

// RUN: %clang -target x86_64-unknown-linux-gnu -mamx-avx512 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-AVX512 %s
// RUN: %clang -target x86_64-unknown-linux-gnu -mno-amx-avx512 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-AVX512 %s
// AMX-AVX512: "-target-feature" "+amx-avx512"
// NO-AMX-AVX512: "-target-feature" "-amx-avx512"
#endif // INTEL_FEATURE_ISA_AMX_AVX512