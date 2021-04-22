#if INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
// REQUIRES: intel_feature_isa_amx_avx512_cvtrow

// RUN: %clang -target x86_64-unknown-linux-gnu -mamx-avx512-cvtrow %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-AVX512-CVTROW %s
// RUN: %clang -target x86_64-unknown-linux-gnu -mno-amx-avx512-cvtrow %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-AVX512-CVTROW %s
// AMX-AVX512-CVTROW: "-target-feature" "+amx-avx512-cvtrow"
// NO-AMX-AVX512-CVTROW: "-target-feature" "-amx-avx512-cvtrow"
#endif // INTEL_FEATURE_ISA_AMX_AVX512_CVTROW