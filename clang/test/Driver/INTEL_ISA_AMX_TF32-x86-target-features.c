#if INTEL_FEATURE_ISA_AMX_TF32
// REQUIRES: intel_feature_isa_amx_tf32

// RUN: %clang -target x86_64-unknown-linux-gnu -mamx-tf32 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-TF32 %s
// RUN: %clang -target x86_64-unknown-linux-gnu -mno-amx-tf32 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-TF32 %s
// AMX-TF32: "-target-feature" "+amx-tf32"
// NO-AMX-TF32: "-target-feature" "-amx-tf32"
#endif // INTEL_FEATURE_ISA_AMX_TF32