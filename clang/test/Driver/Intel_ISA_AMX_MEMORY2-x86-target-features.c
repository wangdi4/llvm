#if INTEL_FEATURE_ISA_AMX_MEMORY2
// REQUIRES: intel_feature_isa_amx_memory2

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-memory2 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-MEMORY2 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-memory2 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-MEMORY2 %s
// AMX-MEMORY2: "-target-feature" "+amx-memory2"
// NO-AMX-MEMORY2: "-target-feature" "-amx-memory2"

#endif // INTEL_FEATURE_ISA_AMX_MEMORY2
