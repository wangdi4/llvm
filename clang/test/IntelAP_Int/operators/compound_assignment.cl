// RUN: %clang -cc1 -O3 -disable-llvm-passes %s -emit-llvm -o %t
// RUN: FileCheck %s < %t
// CHECK: %[[L0:[0-9]+]] = load i43, i43* %x43_s, align 8
// CHECK: %[[S0:[a-zA-Z0-9]+]] = sdiv i43 %[[L0]], 7
// CHECK: store i43 %[[S0]], i43* %x43_s, align 8
// CHECK-NOT: = sdiv i32
// CHECK: %[[L1:[0-9]+]] = load i43, i43* %x43_s, align 8
// CHECK: %[[S1:[a-zA-Z0-9]+]] = srem i43 %[[L1]], 7
// CHECK: store i43 %[[S1]], i43* %x43_s, align 8
// CHECK-NOT: = srem i32

// CHECK: %[[L2:[0-9]+]] = load i43, i43* %x43_s, align 8
// CHECK: %[[S2:[a-zA-Z0-9]+]] = ashr i43 %[[L2]], 7
// CHECK: store i43 %[[S2]], i43* %x43_s, align 8
// CHECK-NOT: = ashr i32

// CHECK: %[[L3:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U0:[a-zA-Z0-9]+]] = add i43 %[[L3]], 2
// CHECK: store i43 %[[U0]], i43* %x43_u, align 8
// CHECK-NOT: = add i32
// CHECK: %[[L4:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U1:[a-zA-Z0-9]+]] = sub i43 %[[L4]], 2
// CHECK: store i43 %[[U1]], i43* %x43_u, align 8
// CHECK-NOT: = sub i32
// CHECK: %[[L5:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U2:[a-zA-Z0-9]+]] = mul i43 %[[L5]], 2
// CHECK: store i43 %[[U2]], i43* %x43_u, align 8
// CHECK-NOT: = mul i32
// CHECK: %[[L6:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U3:[a-zA-Z0-9]+]] = udiv i43 %[[L6]], 2
// CHECK: store i43 %[[U3]], i43* %x43_u, align 8
// CHECK-NOT: = udiv i32
// CHECK: %[[L7:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U4:[a-zA-Z0-9]+]] = urem i43 %[[L7]], 2
// CHECK: store i43 %[[U4]], i43* %x43_u, align 8
// CHECK-NOT: = urem i32
// CHECK: %[[L8:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U5:[a-zA-Z0-9]+]] = or i43 %[[L8]], 2
// CHECK: store i43 %[[U5]], i43* %x43_u, align 8
// CHECK-NOT: = or i32
// CHECK: %[[L9:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U6:[a-zA-Z0-9]+]] = and i43 %[[L9]], 2
// CHECK: store i43 %[[U6]], i43* %x43_u, align 8
// CHECK-NOT: = and i32
// CHECK: %[[L10:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U7:[a-zA-Z0-9]+]] = xor i43 %[[L10]], 2
// CHECK: store i43 %[[U7]], i43* %x43_u, align 8
// CHECK-NOT: = xor i32

// CHECK: %[[L11:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U8:[a-zA-Z0-9]+]] = lshr i43 %[[L11]], 2
// CHECK: store i43 %[[U8]], i43* %x43_u, align 8
// CHECK-NOT: = lshr i32
// CHECK: %[[L12:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U9:[a-zA-Z0-9]+]] = shl i43 %[[L12]], 2
// CHECK: store i43 %[[U9]], i43* %x43_u, align 8
// CHECK-NOT: = shl i32

#include "../ihc_apint.h"

kernel void foo() {
  int43_tt x43_s = 0;
  int4_tt x4_s = 0;
  uint43_tt x43_u = 0;
  uint4_tt x4_u = 0;

  // signed (only those that differ from unsigned)
  x43_s /= 7;
  x4_s /= (int4_tt) 7;
  x43_s %= 7;
  x4_s %= (int4_tt) 7;

  x43_s >>= 7;
  x4_s >>= (int4_tt) 7;

  // unsigned
  x43_u += 2;
  x4_u += (uint4_tt) 2;
  x43_u -= 2;
  x4_u -= (uint4_tt) 2;
  x43_u *= 2;
  x4_u *= (uint4_tt) 2;
  x43_u /= 2;
  x4_u /= (uint4_tt) 2;

  x43_u %= 2;
  x4_u %= (uint4_tt) 2;
  x43_u |= 2;
  x4_u |= (uint4_tt) 2;
  x43_u &= 2;
  x4_u &= (uint4_tt) 2;
  x43_u ^= 2;
  x4_u ^= (uint4_tt) 2;

  x43_u >>= 2;
  x4_u >>= (uint4_tt) 2;
  x43_u <<= 2;
  x4_u <<= (uint4_tt) 2;
}