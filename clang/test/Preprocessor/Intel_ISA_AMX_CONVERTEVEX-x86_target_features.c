#if INTEL_FEATURE_ISA_AMX_CONVERT_EVEX
// REQUIRES: intel_feature_isa_amx_convert_evex

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-convert-evex -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-CONVERT-EVEX %s
// AMX-CONVERT-EVEX: #define __AMX_CONVERTEVEX__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-convert-evex -x \
// RUN: c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-CONVERT-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-convert-evex \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-CONVERT-EVEX %s
// NO-AMX-CONVERT-EVEX-NOT: #define __AMX_CONVERTEVEX__ 1

#endif // INTEL_FEATURE_ISA_AMX_CONVERT_EVEX
