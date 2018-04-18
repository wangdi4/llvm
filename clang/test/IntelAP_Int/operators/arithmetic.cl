// RUN: %clang -cc1 -O3 -disable-llvm-passes -fhls -x cl %s -emit-llvm -o %t
// RUN: FileCheck %s < %t
// CHECK: %[[L0:[0-9]+]] = load i43, i43* %x43_s, align 8
// CHECK: %[[L1:[0-9]+]] = load i43, i43* %y43_s, align 8
// CHECK: = sdiv i43 %[[L0]], %[[L1]]
// CHECK-NOT: = sdiv i32
// CHECK: %[[L2:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[L3:[0-9]+]] = load i43, i43* %y43_u, align 8
// CHECK: = add i43 %[[L2]], %[[L3]]
// CHECK-NOT: = add i32
// CHECK: %[[L4:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[L5:[0-9]+]] = load i43, i43* %y43_u, align 8
// CHECK: = sub i43 %[[L4]], %[[L5]]
// CHECK-NOT: = sub i32
// CHECK: %[[L6:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[L7:[0-9]+]] = load i43, i43* %y43_u, align 8
// CHECK: = mul i43 %[[L6]], %[[L7]]
// CHECK-NOT: = mul i32
// CHECK: %[[L8:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[L9:[0-9]+]] = load i43, i43* %y43_u, align 8
// CHECK: = udiv i43 %[[L8]], %[[L9]]
// CHECK-NOT: = udiv i32

#include "../ihc_apint.h"

kernel void foo() {
  int43_tt x43_s = 1, y43_s = 1;
  int4_tt x4_s = 1, y4_s = 1;
  uint43_tt x43_u = 1, y43_u = 1;
  uint4_tt x4_u = 1, y4_u = 1;
  long res = 0;

  // signed (only those that differ from unsigned)
  res = x43_s / y43_s;
  res = x4_s / y4_s;

  // unsigned
  res = x43_u + y43_u;
  res = x4_u + y4_u;
  res = x43_u - y43_u;
  res = x4_u - y4_u;
  res = x43_u * y43_u;
  res = x4_u * y4_u;
  res = x43_u / y43_u;
  res = x4_u / y4_u;
}
