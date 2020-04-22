#if INTEL_FEATURE_ISA_AMX_INT8_EVEX
// REQUIRES: intel_feature_isa_amx_int8_evex

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-int8-evex %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-INT8-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-int8-evex \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-INT8-EVEX %s
// AMX-INT8-EVEX: "-target-feature" "+amx-int8-evex"
// NO-AMX-INT8-EVEX: "-target-feature" "-amx-int8-evex"

#endif // INTEL_FEATURE_ISA_AMX_INT8_EVEX
