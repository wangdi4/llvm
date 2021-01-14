// INTEL UNSUPPORTED: intel_opencl && i686-pc-windows
// RUN: %clang -cc1 -O3 -disable-llvm-passes %s -emit-llvm -o %t
// RUN: FileCheck %s < %t
// CHECK: store i43 8, i43* %x43_u, align 8
// CHECK: store i17 8, i17* %x17_u, align 4

// CHECK: %[[L0:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U0:[a-zA-Z0-9]+]] = zext i43 %[[L0]] to i64
// CHECK: store i64 %[[U0]], i64* %res, align 8
// CHECK: %[[L00:[0-9]+]] = load i17, i17* %x17_u, align 4
// CHECK: %[[U00:[a-zA-Z0-9]+]] = zext i17 %[[L00]] to i64
// CHECK: store i64 %[[U00]], i64* %res, align 8
// CHECK: %[[L1:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U1:[a-zA-Z0-9]+]] = sub i43 0, %[[L1]]
// CHECK: %[[U2:[a-zA-Z0-9]+]] = zext i43 %[[U1]] to i64
// CHECK: store i64 %[[U2]], i64* %res, align 8
// CHECK: %[[L10:[0-9]+]] = load i17, i17* %x17_u, align 4
// CHECK: %[[U10:[a-zA-Z0-9]+]] = sub i17 0, %[[L10]]
// CHECK: %[[U20:[a-zA-Z0-9]+]] = zext i17 %[[U10]] to i64
// CHECK: store i64 %[[U20]], i64* %res, align 8

// CHECK: %[[L2:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U3:[a-zA-Z0-9]+]] = icmp ne i43 %[[L2]], 0
// CHECK: %[[U4:[a-zA-Z0-9]+]] = xor i1 %[[U3]], true
// CHECK: %[[U5:[a-zA-Z0-9.]+]] = zext i1 %[[U4]] to i43
// CHECK: %[[U6:[a-zA-Z0-9]+]] = zext i43 %[[U5]] to i64
// CHECK: store i64 %[[U6]], i64* %res, align 8
// CHECK: %[[L20:[0-9]+]] = load i17, i17* %x17_u, align 4
// CHECK: %[[U30:[a-zA-Z0-9]+]] = icmp ne i17 %[[L20]], 0
// CHECK: %[[U40:[a-zA-Z0-9]+]] = xor i1 %[[U30]], true
// CHECK: %[[U50:[a-zA-Z0-9.]+]] = zext i1 %[[U40]] to i17
// CHECK: %[[U60:[a-zA-Z0-9]+]] = zext i17 %[[U50]] to i64
// CHECK: store i64 %[[U60]], i64* %res, align 8

// CHECK: store i64 8, i64* %res, align 8
// CHECK: store i64 4, i64* %res, align 8

// CHECK: store i43* %x43_u, i43** %p, align 8
// CHECK: %[[L3:[0-9]+]] = load i43*, i43** %p, align 8
// CHECK: %[[L4:[0-9]+]] = load i43, i43* %[[L3]], align 8
// CHECK: store i43 %[[L4]], i43* %y43_u, align 8
// CHECK: store i17* %x17_u, i17** %p2, align 8
// CHECK: %[[L30:[0-9]+]] = load i17*, i17** %p2, align 8
// CHECK: %[[L40:[0-9]+]] = load i17, i17* %[[L30]], align 4
// CHECK: store i17 %[[L40]], i17* %y17_u, align 4

#include "../ihc_apint.h"

kernel void foo() {
  uint43_tt x43_u = 0, y43_u = 0;
  uint17_tt x17_u = 0, y17_u = 0;
  long res = 0;

  // signed (only those that differ from unsigned)
  /* none */

  // unsigned
  x43_u = 8;
  x17_u = 8;

  res = +x43_u;
  res = +x17_u;
  res = -x43_u;
  res = -x17_u;

  res = !x43_u;
  res = !x17_u;

  res = sizeof(uint43_tt);
  res = sizeof(uint17_tt);

  uint43_tt *p = &x43_u;
  y43_u = *p;
  uint17_tt *p2 = &x17_u;
  y17_u = *p2;
}
