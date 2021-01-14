#if INTEL_FEATURE_ISA_AMX_FP19
// REQUIRES: intel_feature_isa_amx_fp19

// RUN: %clang -target x86_64-unknown-linux-gnu -mamx-fp19 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-FP19 %s
// RUN: %clang -target x86_64-unknown-linux-gnu -mno-amx-fp19 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-FP19 %s
// AMX-FP19: "-target-feature" "+amx-fp19"
// NO-AMX-FP19: "-target-feature" "-amx-fp19"
#endif // INTEL_FEATURE_ISA_AMX_FP19