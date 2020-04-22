#if INTEL_FEATURE_ISA_AMX_MEMORY2
// REQUIRES: intel_feature_isa_amx_memory2

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-memory2 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-MEMORY2 %s
// AMX-MEMORY2: #define __AMX_MEMORY2__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-memory2 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-MEMORY2 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-memory2 \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-MEMORY2 %s
// NO-AMX-MEMORY2-NOT: #define __AMX_MEMORY2__ 1

#endif // INTEL_FEATURE_ISA_AMX_MEMORY2
