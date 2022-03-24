// Test /Qpc and -pc behaviors
// REQUIRES: x86-registered-target

// RUN: %clang -### -pc80 %s 2>&1 | FileCheck -check-prefix=CHECK_PC80 %s
// RUN: %clang_cl -### /Qpc80 %s 2>&1 | FileCheck -check-prefix=CHECK_PC80 %s
// RUN: %clang -### -pc 80 %s 2>&1 | FileCheck -check-prefix=CHECK_PC80 %s
// CHECK_PC80: "-mx87-precision=80"

// RUN: %clang -### -pc64 %s 2>&1 | FileCheck -check-prefix=CHECK_PC64 %s
// RUN: %clang_cl -### /Qpc64 %s 2>&1 | FileCheck -check-prefix=CHECK_PC64 %s
// RUN: %clang_cl -### /Qpc 64 %s 2>&1 | FileCheck -check-prefix=CHECK_PC64 %s
// CHECK_PC64: "-mx87-precision=64"

// RUN: %clang -### -pc32 %s 2>&1 | FileCheck -check-prefix=CHECK_PC32 %s
// RUN: %clang -### -pc=32 %s 2>&1 | FileCheck -check-prefix=CHECK_PC32 %s
// RUN: %clang_cl -### /Qpc32 %s 2>&1 | FileCheck -check-prefix=CHECK_PC32 %s
// RUN: %clang_cl -### /Qpc:32 %s 2>&1 | FileCheck -check-prefix=CHECK_PC32 %s
// RUN: %clang_cl -### /Qpc=32 %s 2>&1 | FileCheck -check-prefix=CHECK_PC32 %s
// CHECK_PC32: "-mx87-precision=32"

// RUN: %clang -### -pc3 %s 2>&1 | FileCheck -check-prefix=CHECK_PC_UNKNOWN %s
// RUN: %clang -### -pc 3 %s 2>&1 | FileCheck -check-prefix=CHECK_PC_UNKNOWN %s
// RUN: %clang_cl -### /Qpc3 %s 2>&1 | FileCheck -check-prefix=CHECK_PC_UNKNOWN %s
// RUN: %clang_cl -### /Qpc 3 %s 2>&1 | FileCheck -check-prefix=CHECK_PC_UNKNOWN %s
// CHECK_PC_UNKNOWN: unsupported argument
// CHECK_PC_UNKNOWN-NOT: "-mx87-precision=3"

