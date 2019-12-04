#if INTEL_FEATURE_ISA_AMX_FUTURE
// REQUIRES: intel_feature_isa_amx_future

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-reduce -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-REDUCE %s
// AMX-REDUCE: #define __AMXREDUCE__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-reduce -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-REDUCE %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-reduce \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-REDUCE %s
// NO-AMX-REDUCE-NOT: #define __AMXREDUCE__ 1

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-memory -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-MEMORY %s
// AMX-MEMORY: #define __AMXMEMORY__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-memory -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-MEMORY %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-memory \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-MEMORY %s
// NO-AMX-MEMORY-NOT: #define __AMXMEMORY__ 1

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-format -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-FORMAT %s
// AMX-FORMAT: #define __AMXFORMAT__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-format -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-FORMAT %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-format \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-FORMAT %s
// NO-AMX-FORMAT-NOT: #define __AMXFORMAT__ 1

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-element -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-ELEMENT %s
// AMX-ELEMENT: #define __AMXELEMENT__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-element -x \
// RUN: c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-ELEMENT %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-element \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-ELEMENT %s
// NO-AMX-ELEMENT-NOT: #define __AMXELEMENT__ 1
#endif // INTEL_FEATURE_ISA_AMX_FUTURE
