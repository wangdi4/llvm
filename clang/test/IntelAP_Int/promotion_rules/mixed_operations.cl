// Ensure that promotion rules are obeyed

// RUN: %clang -cc1 -O3 -disable-llvm-passes %s -emit-llvm -o %t
// RUN: FileCheck %s < %t
// CHECK: %[[L0:[0-9]+]] = load i15, i15* %x15_u, align 2
// CHECK: %[[M0:[a-zA-Z0-9]+]] = zext i15 %[[L0]] to i16
// CHECK: %[[L1:[0-9]+]] = load i16, i16* %s, align 2
// CHECK: %[[M1:[a-zA-Z0-9]+]] = add nsw i16 %[[M0]], %[[L1]]
// CHECK: %[[L2:[0-9]+]] = load i8, i8* %c, align 1
// CHECK: %[[M2:[a-zA-Z0-9]+]] = sext i8 %[[L2]] to i16
// CHECK: %[[M3:[a-zA-Z0-9]+]] = mul nsw i16 %[[M1]], %[[M2]]
// CHECK: %[[M4:[a-zA-Z0-9]+]] = sext i16 %[[M3]] to i64
// CHECK: store i64 %[[M4]], i64* %res, align 8

#include "../ihc_apint.h"

kernel void foo() {
  uint15_tt x15_u = 0;

  char c = '0';
  short s = 0;
  long res = 0;

  res = (x15_u + s) * c;
}
