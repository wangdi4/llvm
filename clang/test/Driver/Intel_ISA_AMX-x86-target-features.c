#if INTEL_FEATURE_ISA_AMX
// REQUIRES: intel_feature_isa_amx
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-tile %s -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-TILE %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-tile %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-TILE %s
// AMX-TILE: "-target-feature" "+amx-tile"
// NO-AMX-TILE: "-target-feature" "-amx-tile"

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-bf16 %s -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-BF16 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-bf16 %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-BF16 %s
// AMX-BF16: "-target-feature" "+amx-bf16"
// NO-AMX-BF16: "-target-feature" "-amx-bf16"

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-int8 %s -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-INT8 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-int8 %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-INT8 %s
// AMX-INT8: "-target-feature" "+amx-int8"
// NO-AMX-INT8: "-target-feature" "-amx-int8"
#endif // INTEL_FEATURE_ISA_AMX
