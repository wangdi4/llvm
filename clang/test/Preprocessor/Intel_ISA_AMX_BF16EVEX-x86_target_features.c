#if INTEL_FEATURE_ISA_AMX_BF16_EVEX
// REQUIRES: intel_feature_isa_amx_bf16_evex

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-bf16-evex -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-BF16-EVEX %s
// AMX-BF16-EVEX: #define __AMX_BF16EVEX__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-bf16-evex -x \
// RUN: c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-BF16-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-bf16-evex \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-BF16-EVEX %s
// NO-AMX-BF16-EVEX-NOT: #define __AMX_BF16EVEX__ 1

#endif // INTEL_FEATURE_ISA_AMX_BF16_EVEX
