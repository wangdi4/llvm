// Ensure the correct promotion rules are followed in operations with 2 ap_[u]int types
// RUN: %clang -cc1 -O3 -disable-llvm-passes %s -emit-llvm -o %t
// RUN: FileCheck %s < %t
// CHECK: %[[L0:[0-9]+]] = load i17, i17* %x17_u, align 4
// CHECK: %[[L1:[0-9]+]] = load i17, i17* %x17_u, align 4
// CHECK: = urem i17 %[[L0]], %[[L1]]
// CHECK: %[[L2:[0-9]+]] = load i17, i17* %x17_u, align 4
// CHECK: %[[L3:[0-9]+]] = load i17, i17* %x17_s, align 4
// CHECK: = urem i17 %[[L2]], %[[L3]]
// CHECK: %[[L4:[0-9]+]] = load i17, i17* %x17_s, align 4
// CHECK: %[[L5:[0-9]+]] = load i17, i17* %x17_s, align 4
// CHECK: = srem i17 %[[L4]], %[[L5]]

// CHECK: %[[L6:[0-9]+]] = load i17, i17* %x17_u, align 4
// CHECK: %[[L7:[0-9]+]] = load i15, i15* %x15_u, align 2
// CHECK: %[[M0:[a-zA-Z0-9]+]] = zext i15 %[[L7]] to i17
// CHECK: = urem i17 %[[L6]], %[[M0]]
// CHECK: %[[L8:[0-9]+]] = load i17, i17* %x17_u, align 4
// CHECK: %[[L9:[0-9]+]] = load i15, i15* %x15_s, align 2
// CHECK: %[[M1:[a-zA-Z0-9]+]] = sext i15 %[[L9]] to i17
// CHECK: = urem i17 %[[L8]], %[[M1]]
// CHECK: %[[L10:[0-9]+]] = load i17, i17* %x17_s, align 4
// CHECK: %[[L11:[0-9]+]] = load i15, i15* %x15_u, align 2
// CHECK: %[[M2:[a-zA-Z0-9]+]] = zext i15 %[[L11]] to i17
// CHECK: = srem i17 %[[L10]], %[[M2]]
// CHECK: %[[L12:[0-9]+]] = load i17, i17* %x17_s, align 4
// CHECK: %[[L13:[0-9]+]] = load i15, i15* %x15_s, align 2
// CHECK: %[[M3:[a-zA-Z0-9]+]] = sext i15 %[[L13]] to i17
// CHECK: = srem i17 %[[L12]], %[[M3]]

#include "../../ihc_apint.h"

kernel void foo() {
  int15_tt x15_s = 0;
  uint15_tt x15_u = 0;

  int17_tt x17_s = 0;
  uint17_tt x17_u = 0;

  long res = 0;

  // 2 ap (SAME BEHAVIOR IF >= 32 bits)
  // same size
  res = x17_u % x17_u;
  res = x17_u % x17_s;
  res = x17_s % x17_s;

  // different size
  res = x17_u % x15_u;
  res = x17_u % x15_s;
  res = x17_s % x15_u;
  res = x17_s % x15_s;
}
