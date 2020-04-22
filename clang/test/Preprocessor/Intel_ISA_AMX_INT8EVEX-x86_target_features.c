#if INTEL_FEATURE_ISA_AMX_INT8_EVEX
// REQUIRES: intel_feature_isa_amx_int8_evex

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-int8-evex -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-INT8-EVEX %s
// AMX-INT8-EVEX: #define __AMX_INT8EVEX__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-int8-evex -x \
// RUN: c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-INT8-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-int8-evex \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-INT8-EVEX %s
// NO-AMX-INT8-EVEX-NOT: #define __AMX_INT8EVEX__ 1

#endif // INTEL_FEATURE_ISA_AMX_INT8_EVEX
