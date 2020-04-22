#if INTEL_FEATURE_ISA_AMX_TILE_EVEX
// REQUIRES: intel_feature_isa_amx_tile_evex

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-tile-evex -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-TILE-EVEX %s
// AMX-TILE-EVEX: #define __AMX_TILEEVEX__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-tile-evex -x \
// RUN: c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-TILE-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-tile-evex \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-TILE-EVEX %s
// NO-AMX-TILE-EVEX-NOT: #define __AMX_TILEEVEX__ 1

#endif // INTEL_FEATURE_ISA_AMX_TILE_EVEX
