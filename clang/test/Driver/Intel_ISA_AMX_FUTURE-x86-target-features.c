#if INTEL_FEATURE_ISA_AMX_FUTURE
// REQUIRES: intel_feature_isa_amx_future

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-reduce %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-REDUCE %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-reduce \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-REDUCE %s
// AMX-REDUCE: "-target-feature" "+amx-reduce"
// NO-AMX-REDUCE: "-target-feature" "-amx-reduce"

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-memory %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-MEMORY %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-memory \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-MEMORY %s
// AMX-MEMORY: "-target-feature" "+amx-memory"
// NO-AMX-MEMORY: "-target-feature" "-amx-memory"

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-format %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-FORMAT %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-format \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-FORMAT %s
// AMX-FORMAT: "-target-feature" "+amx-format"
// NO-AMX-FORMAT: "-target-feature" "-amx-format"

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-element %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-ELEMENT %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-element \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-ELEMENT %s
// AMX-ELEMENT: "-target-feature" "+amx-element"
// NO-AMX-ELEMENT: "-target-feature" "-amx-element"
#endif // INTEL_FEATURE_ISA_AMX_FUTURE
