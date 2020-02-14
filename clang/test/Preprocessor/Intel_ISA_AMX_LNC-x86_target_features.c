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

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-bf16-evex -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-BF16-EVEX %s
// AMX-BF16-EVEX: #define __AMXBF16EVEX__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-bf16-evex -x \
// RUN: c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-BF16-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-bf16-evex \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-BF16-EVEX %s
// NO-AMX-BF16-EVEX-NOT: #define __AMXBF16EVEX__ 1

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-int8-evex -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-INT8-EVEX %s
// AMX-INT8-EVEX: #define __AMXINT8EVEX__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-int8-evex -x \
// RUN: c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-INT8-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-int8-evex \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-INT8-EVEX %s
// NO-AMX-INT8-EVEX-NOT: #define __AMXINT8EVEX__ 1

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-tile-evex -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-TILE-EVEX %s
// AMX-TILE-EVEX: #define __AMXTILEEVEX__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-tile-evex -x \
// RUN: c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-TILE-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-tile-evex \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-TILE-EVEX %s
// NO-AMX-TILE-EVEX-NOT: #define __AMXTILEEVEX__ 1

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-element-evex -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-ELEMENT-EVEX %s
// AMX-ELEMENT-EVEX: #define __AMXELEMENTEVEX__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-element-evex -x \
// RUN: c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-ELEMENT-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-element-evex \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-ELEMENT-EVEX %s
// NO-AMX-ELEMENT-EVEX-NOT: #define __AMXELEMENTEVEX__ 1

#endif // INTEL_FEATURE_ISA_AMX_LNC
