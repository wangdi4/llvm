#if INTEL_FEATURE_ISA_AMX_LNC
// REQUIRES: intel_feature_isa_amx_lnc

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-transpose -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-TRANSPOSE %s
// AMX-TRANSPOSE: #define __AMXTRANSPOSE__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-transpose \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-TRANSPOSE %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-transpose \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-TRANSPOSE %s
// NO-AMX-TRANSPOSE-NOT: #define __AMXTRANSPOSE__ 1

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-avx512 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-AVX512 %s
// AMX-AVX512: #define __AMXAVX512__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-avx512 -x \
// RUN: c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-AVX512 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-avx512 \
// RUN: -mno-amx-avx512 -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-AVX512 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-avx512 \
// RUN: -mno-avx512f -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-AVX512 %s
// NO-AMX-AVX512-NOT: #define __AMXAVX512__ 1

#endif // INTEL_FEATURE_ISA_AMX_LNC
