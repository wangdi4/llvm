#if INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
// REQUIRES: intel_feature_isa_amx_element_evex

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-element-evex %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-ELEMENT-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-element-evex \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-ELEMENT-EVEX %s
// AMX-ELEMENT-EVEX: "-target-feature" "+amx-element-evex"
// NO-AMX-ELEMENT-EVEX: "-target-feature" "-amx-element-evex"

#endif // INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
