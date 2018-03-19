// RUN: %clang -cc1 -O3 -disable-llvm-passes %s -emit-llvm -o %t
// RUN: FileCheck %s --implicit-check-not "add {{.*}} i32" < %t

// CHECK: %[[L0:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U0:[a-zA-Z0-9]+]] = add i43 %[[L0]], 1
// CHECK: store i43 %[[U0]], i43* %x43_u, align 8
// CHECK: %[[U1:[a-zA-Z0-9]+]] = zext i43 %[[U0]] to i64
// CHECK: store i64 %[[U1]], i64* %res, align 8

// CHECK: %[[L1:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U2:[a-zA-Z0-9]+]] = add i43 %[[L1]], 1
// CHECK: store i43 %[[U2]], i43* %x43_u, align 8
// CHECK: %[[U3:[a-zA-Z0-9]+]] = zext i43 %[[L1]] to i64
// CHECK: store i64 %[[U3]], i64* %res, align 8

// CHECK: %[[L2:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U4:[a-zA-Z0-9]+]] = add i43 %[[L2]], -1
// CHECK: store i43 %[[U4]], i43* %x43_u, align 8
// CHECK: %[[U5:[a-zA-Z0-9]+]] = zext i43 %[[U4]] to i64
// CHECK: store i64 %[[U5]], i64* %res, align 8

// CHECK: %[[L3:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U6:[a-zA-Z0-9]+]] = add i43 %[[L3]], -1
// CHECK: store i43 %[[U6]], i43* %x43_u, align 8
// CHECK: %[[U7:[a-zA-Z0-9]+]] = zext i43 %[[L3]] to i64
// CHECK: store i64 %[[U7]], i64* %res, align 8

#include "../ihc_apint.h"

kernel void foo() {
  uint43_tt x43_u = 0;
  uint4_tt x4_u = 0;
  long res = 0;

  // signed (only those that differ from unsigned)
  /* none */

  // unsigned
  res = ++x43_u;
  res = ++x4_u;
  res = x43_u++;
  res = x4_u++;
  res = --x43_u;
  res = --x4_u;
  res = x43_u--;
  res = x4_u--;
}