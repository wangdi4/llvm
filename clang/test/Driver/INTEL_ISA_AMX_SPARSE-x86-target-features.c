#if INTEL_FEATURE_ISA_AMX_SPARSE
// REQUIRES: intel_feature_isa_amx_sparse

// RUN: %clang -target x86_64-unknown-linux-gnu -mamx-sparse %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-SPARSE %s
// RUN: %clang -target x86_64-unknown-linux-gnu -mno-amx-sparse %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-SPARSE %s
// AMX-SPARSE: "-target-feature" "+amx-sparse"
// NO-AMX-SPARSE: "-target-feature" "-amx-sparse"
#endif // INTEL_FEATURE_ISA_AMX_SPARSE