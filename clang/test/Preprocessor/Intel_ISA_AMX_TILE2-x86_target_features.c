#if INTEL_FEATURE_ISA_AMX_TILE2
// REQUIRES: intel_feature_isa_amx_tile2

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-tile2 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-TILE2 %s
// AMX-TILE2: #define __AMX_TILE2__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-tile2 \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-TILE2 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-tile2 \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-TILE2 %s
// NO-AMX-TILE2-NOT: #define __AMX_TILE2__ 1

#endif // INTEL_FEATURE_ISA_AMX_TILE2
