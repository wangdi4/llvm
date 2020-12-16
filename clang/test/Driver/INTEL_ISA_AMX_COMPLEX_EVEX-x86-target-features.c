#if INTEL_FEATURE_ISA_AMX_COMPLEX_EVEX
// REQUIRES: intel_feature_isa_amx_complex_evex

// RUN: %clang -target x86_64-unknown-linux-gnu -mamx-complex-evex %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-COMPLEX-EVEX %s
// RUN: %clang -target x86_64-unknown-linux-gnu -mno-amx-complex-evex %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-COMPLEX-EVEX %s
// AMX-COMPLEX-EVEX: "-target-feature" "+amx-complex-evex"
// NO-AMX-COMPLEX-EVEX: "-target-feature" "-amx-complex-evex"
#endif // INTEL_FEATURE_ISA_AMX_COMPLEX_EVEX