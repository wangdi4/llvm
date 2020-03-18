#if INTEL_FEATURE_ISA_AMX_FP16
// REQUIRES: intel_feature_isa_amx_fp16

// RUN: %clang -target x86_64-unknown-linux-gnu -march=atom -mamx-fp16 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-FP16 %s

// AMX-FP16: #define __AMXFP16__ 1
// AMX-FP16: #define __AMXTILE__ 1

// RUN: %clang -target x86_64-unknown-linux-gnu -march=atom -mno-amx-fp16 \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-FP16 %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=atom -mamx-fp16 \
// RUN: -mno-amx-tile -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-FP16 %s

// NO-AMX-FP16-NOT: #define __AMXFP16__ 1
// NO-AMX-FP16-NOT: #define __AMXTILE__ 1

#endif // INTEL_FEATURE_ISA_AMX_FP16
