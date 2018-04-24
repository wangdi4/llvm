// Ensure the correct promotion rules are followed when literals are used in an expression with ap_[u]ints
// RUN: %clang -cc1 -O3 -disable-llvm-passes %s -emit-llvm -o %t
// RUN: FileCheck %s < %t
// CHECK: %[[L0:[0-9]+]] = load i7, i7* %x7_u, align 1
// CHECK: %[[M0:[a-zA-Z0-9]+]] = zext i7 %[[L0]] to i32
// CHECK: = xor i32 %[[M0]], 5
// CHECK: %[[L1:[0-9]+]] = load i7, i7* %x7_u, align 1
// CHECK: %[[M1:[a-zA-Z0-9]+]] = zext i7 %[[L1]] to i32
// CHECK: = xor i32 %[[M1]], 5
// CHECK: %[[L2:[0-9]+]] = load i7, i7* %x7_u, align 1
// CHECK: %[[M2:[a-zA-Z0-9]+]] = zext i7 %[[L2]] to i64
// CHECK: = xor i64 %[[M2]], 5
// CHECK: %[[L3:[0-9]+]] = load i7, i7* %x7_u, align 1
// CHECK: = xor i7 %[[L3]], 5

#include "../../ihc_apint.h"

kernel void foo() {
  uint7_tt x7_u = 0;
  long res = 0;

  // signed (only those that differ from unsigned)
  /*none*/

  // unsigned
  res = x7_u ^ 5;
  res = x7_u ^ 5U;
  res = x7_u ^ 5L;
  res = x7_u ^ (uint4_tt) 5;
}
