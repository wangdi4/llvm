// RUN: not %clang -fsyntax-only %s 2>&1 | FileCheck %s

#pragma clang __debug parser_crash

// CHECK-NOT: bugs.llvm.org
// CHECK: PLEASE submit a bug report to https://software.intel.com/en-us/support/priority-support
// CHECK-NOT: bugs.llvm.org
