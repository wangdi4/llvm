#if INTEL_FEATURE_ISA_AMX_TRANSPOSE2
// REQUIRES: intel_feature_isa_amx_transpose2

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-transpose2 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-TRANSPOSE2 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-transpose2 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-TRANSPOSE2 %s
// AMX-TRANSPOSE2: "-target-feature" "+amx-transpose2"
// NO-AMX-TRANSPOSE2: "-target-feature" "-amx-transpose2"

#endif // INTEL_FEATURE_ISA_AMX_TRANSPOSE2
