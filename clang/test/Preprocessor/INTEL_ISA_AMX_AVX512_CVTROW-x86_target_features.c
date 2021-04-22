#if INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
// REQUIRES: intel_feature_isa_amx_avx512_cvtrow

// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-avx512-cvtrow -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-AVX512-CVTROW %s
// AMX-AVX512-CVTROW: #define __AMXAVX512CVTROW__ 1
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mno-amx-avx512-cvtrow -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-AVX512-CVTROW %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-avx512-cvtrow -mno-amx-tile \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-AVX512-CVTROW %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-avx512-cvtrow -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-AVX512-CVTROW %s
// NO-AMX-AVX512-CVTROW-NOT: #define __AMXAVX512CVTROW__ 1

#endif // INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
