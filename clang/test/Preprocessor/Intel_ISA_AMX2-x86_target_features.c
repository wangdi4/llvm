#if INTEL_FEATURE_ISA_AMX2
// REQUIRES: intel_feature_isa_amx2

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-transpose -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-TRANSPOSE %s
// AMX-TRANSPOSE: #define __AMXTRANSPOSE__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-transpose \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-TRANSPOSE %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-transpose \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-TRANSPOSE %s
// NO-AMX-TRANSPOSE-NOT: #define __AMXTRANSPOSE__ 1

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-reduce -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-REDUCE %s
// AMX-REDUCE: #define __AMXREDUCE__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-reduce -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-REDUCE %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-reduce \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-REDUCE %s
// NO-AMX-REDUCE-NOT: #define __AMXREDUCE__ 1

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-memory -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-MEMORY %s
// AMX-MEMORY: #define __AMXMEMORY__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-memory -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-MEMORY %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-memory \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-MEMORY %s
// NO-AMX-MEMORY-NOT: #define __AMXMEMORY__ 1

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-format -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-FORMAT %s
// AMX-FORMAT: #define __AMXFORMAT__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-format -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-FORMAT %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-format \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-FORMAT %s
// NO-AMX-FORMAT-NOT: #define __AMXFORMAT__ 1

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-element -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-ELEMENT %s
// AMX-ELEMENT: #define __AMXELEMENT__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-element -x \
// RUN: c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-ELEMENT %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-element \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-ELEMENT %s
// NO-AMX-ELEMENT-NOT: #define __AMXELEMENT__ 1

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-fp16 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-FP16 %s
// AMX-FP16: #define __AMXFP16__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-fp16 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-FP16 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-fp16 \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-FP16 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-fp16 \
// RUN: -mno-avx512fp16 -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-FP16 %s
// NO-AMX-FP16-NOT: #define __AMXFP16__ 1

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

#endif // INTEL_FEATURE_ISA_AMX2
