// RUN: %clang -cc1 -O3 -disable-llvm-passes %s -emit-llvm -o %t
// RUN: FileCheck %s --implicit-check-not "icmp [[[a-z]+]] i32" < %t
// CHECK: %[[L0:[0-9]+]] = load i43, i43* %x43_s, align 8
// CHECK: %[[L1:[0-9]+]] = load i43, i43* %y43_s, align 8
// CHECK: = srem i43 %[[L0]], %[[L1]]
// CHECK-NOT: srem i32

// CHECK: %[[L2:[0-9]+]] = load i43, i43* %x43_s, align 8
// CHECK: = ashr i43 %[[L2]], 7

// CHECK: %[[L3:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[L4:[0-9]+]] = load i43, i43* %y43_u, align 8
// CHECK: = urem i43 %[[L3]], %[[L4]]
// CHECK-NOT: = urem i32
// CHECK: %[[L5:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[L6:[0-9]+]] = load i43, i43* %y43_u, align 8
// CHECK: = or i43 %[[L5]], %[[L6]]
// CHECK-NOT: = or i32
// CHECK: %[[L7:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[L8:[0-9]+]] = load i43, i43* %y43_u, align 8
// CHECK: = and i43 %[[L7]], %[[L8]]
// CHECK-NOT: = and i32
// CHECK: %[[L9:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[L10:[0-9]+]] = load i43, i43* %y43_u, align 8
// CHECK: = xor i43 %[[L9]], %[[L10]]
// CHECK-NOT: = xor i32
// CHECK: %[[L11:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: = xor i43 %[[L11]], -1
// CHECK-NOT: = xor i32

// CHECK: %[[L12:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: = lshr i43 %[[L12]], 7
// CHECK-NOT: = lshr i32
// CHECK: %[[L13:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: = shl i43 %[[L13]], 7
// CHECK-NOT: = shl i32

#include "../ihc_apint.h"

kernel void foo() {
  int43_tt x43_s = 0, y43_s = 0;
  int4_tt x4_s = 0, y4_s = 0;
  uint43_tt x43_u = 0, y43_u = 0;
  uint4_tt x4_u = 0, y4_u = 0;

  long res = 0;

  // signed (only those that differ from unsigned)
  res = x43_s % y43_s;
  res = x4_s % y4_s;

  res = x43_s >> 7;
  res = x4_s >> (int4_tt) 7;

  // unsigned
  res = x43_u % y43_u;
  res = x4_u % y4_u;
  res = x43_u | y43_u;
  res = x4_u | y4_u;
  res = x43_u & y43_u;
  res = x4_u & y4_u;
  res = x43_u ^ y43_u;
  res = x4_u ^ y4_u;
  res = ~x43_u;
  res = ~x4_u;

  res = x43_u >> 7;
  res = x4_u >> (uint4_tt) 7;
  res = x43_u << 7;
  res = x4_u << (uint4_tt) 7;
}