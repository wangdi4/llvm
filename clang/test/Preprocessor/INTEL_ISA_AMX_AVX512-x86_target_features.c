#if INTEL_FEATURE_ISA_AMX_AVX512
// REQUIRES: intel_feature_isa_amx_avx512

// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-avx512 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-AVX512 %s
// AMX-AVX512: #define __AMXAVX512__ 1
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mno-amx-avx512 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-AVX512 %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-avx512 -mno-amx-tile \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-AVX512 %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-avx512 -mno-avx512f \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-AVX512 %s
// NO-AMX-AVX512-NOT: #define __AMXAVX512__ 1

#endif // INTEL_FEATURE_ISA_AMX_AVX512
