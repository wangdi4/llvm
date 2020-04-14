#if INTEL_FEATURE_ISA_AMX_TILE2
// REQUIRES: intel_feature_isa_amx_tile2

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-tile2 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-TILE2 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-tile2 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-TILE2 %s
// AMX-TILE2: "-target-feature" "+amx-tile2"
// NO-AMX-TILE2: "-target-feature" "-amx-tile2"

#endif // INTEL_FEATURE_ISA_AMX_TILE2
