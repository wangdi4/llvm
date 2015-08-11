// RUN: %clang_cc1 -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

int test() {}
// CHECK-NOT: unreachable

