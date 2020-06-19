#if INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
// REQUIRES: intel_feature_isa_amx_element_evex

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-element-evex -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-ELEMENT-EVEX %s
// AMX-ELEMENT-EVEX: #define __AMX_ELEMENTEVEX__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-element-evex -x \
// RUN: c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-ELEMENT-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-element-evex \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-ELEMENT-EVEX %s
// NO-AMX-ELEMENT-EVEX-NOT: #define __AMX_ELEMENTEVEX__ 1

#endif // INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
