// RUN: %clang_cc1 -fintel-compatibility %s 2>&1 | FileCheck %s

#pragma message("I'm a message")
// Check that a warning is not printed, just a message
// CHECK-NOT: warning:
