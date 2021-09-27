#if INTEL_FEATURE_ISA_AMX_V3
// REQUIRES: intel_feature_isa_amx_v3

// RUN: %clang -target x86_64-unknown-linux-gnu -mamx-v3 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-V3 %s
// RUN: %clang -target x86_64-unknown-linux-gnu -mno-amx-v3 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-V3 %s
// AMX-V3: "-target-feature" "+amx-v3"
// NO-AMX-V3: "-target-feature" "-amx-v3"
#endif // INTEL_FEATURE_ISA_AMX_V3