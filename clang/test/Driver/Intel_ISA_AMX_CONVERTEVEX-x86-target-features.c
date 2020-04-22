#if INTEL_FEATURE_ISA_AMX_CONVERT_EVEX
// REQUIRES: intel_feature_isa_amx_convert_evex

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-convert-evex %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-CONVERT-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-convert-evex \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-CONVERT-EVEX %s
// AMX-CONVERT-EVEX: "-target-feature" "+amx-convert-evex"
// NO-AMX-CONVERT-EVEX: "-target-feature" "-amx-convert-evex"

#endif // INTEL_FEATURE_ISA_AMX_CONVERT_EVEX
