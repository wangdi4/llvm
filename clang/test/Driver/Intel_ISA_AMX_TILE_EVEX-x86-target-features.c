#if INTEL_FEATURE_ISA_AMX_TILE_EVEX
// REQUIRES: intel_feature_isa_amx_tile_evex

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-tile-evex %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-TILE-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-tile-evex \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-TILE-EVEX %s
// AMX-TILE-EVEX: "-target-feature" "+amx-tile-evex"
// NO-AMX-TILE-EVEX: "-target-feature" "-amx-tile-evex"

#endif // INTEL_FEATURE_ISA_AMX_TILE_EVEX
