#if INTEL_FEATURE_ISA_AMX_SPARSE
// REQUIRES: intel_feature_isa_amx_sparse

// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-sparse -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-SPARSE %s
// AMX-SPARSE: #define __AMXSPARSE__ 1
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mno-amx-sparse -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-SPARSE %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86-64 -mamx-sparse -mno-amx-tile \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-SPARSE %s
// NO-AMX-SPARSE-NOT: #define __AMXSPARSE__ 1

#endif // INTEL_FEATURE_ISA_AMX_SPARSE