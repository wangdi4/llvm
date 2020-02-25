#if INTEL_FEATURE_ISA_AMX_TRANSPOSE2
// REQUIRES: intel_feature_isa_amx_transpose2

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-transpose2 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-TRANSPOSE2 %s
// AMX-TRANSPOSE2: #define __AMX_TRANSPOSE2__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-transpose2 \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-TRANSPOSE2 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-transpose2 \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-TRANSPOSE2 %s
// NO-AMX-TRANSPOSE2-NOT: #define __AMX_TRANSPOSE2__ 1

#endif // INTEL_FEATURE_ISA_AMX_TRANSPOSE2
