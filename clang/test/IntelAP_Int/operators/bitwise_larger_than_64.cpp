// Ensure that bitwise operations are allowed to exceed 64 bits for HLS

// RUN: %clang -cc1 -O3 -disable-llvm-passes -fhls %s -emit-llvm -o %t
// RUN: FileCheck %s < %t
// CHECK: %[[L0:[0-9]+]] = load i2053, i2053* %x2053_s, align 8
// CHECK: %[[L1:[0-9]+]] = load i2053, i2053* %y2053_s, align 8
// CHECK: = srem i2053 %[[L0]], %[[L1]]

// CHECK: %[[L2:[0-9]+]] = load i2053, i2053* %x2053_s, align 8
// CHECK: = ashr i2053 %[[L2]], 7

// CHECK: %[[L3:[0-9]+]] = load i2053, i2053* %x2053_u, align 8
// CHECK: %[[L4:[0-9]+]] = load i2053, i2053* %y2053_u, align 8
// CHECK: = urem i2053 %[[L3]], %[[L4]]
// CHECK: %[[L5:[0-9]+]] = load i2053, i2053* %x2053_u, align 8
// CHECK: %[[L6:[0-9]+]] = load i2053, i2053* %y2053_u, align 8
// CHECK: = or i2053 %[[L5]], %[[L6]]
// CHECK: %[[L7:[0-9]+]] = load i2053, i2053* %x2053_u, align 8
// CHECK: %[[L8:[0-9]+]] = load i2053, i2053* %y2053_u, align 8
// CHECK: = and i2053 %[[L7]], %[[L8]]
// CHECK: %[[L9:[0-9]+]] = load i2053, i2053* %x2053_u, align 8
// CHECK: %[[L10:[0-9]+]] = load i2053, i2053* %y2053_u, align 8
// CHECK: = xor i2053 %[[L9]], %[[L10]]
// CHECK: %[[L11:[0-9]+]] = load i2053, i2053* %x2053_u, align 8
// CHECK: = xor i2053 %[[L11]], -1

// CHECK: %[[L12:[0-9]+]] = load i2053, i2053* %x2053_u, align 8
// CHECK: = lshr i2053 %[[L12]], 7
// CHECK: %[[L13:[0-9]+]] = load i2053, i2053* %x2053_u, align 8
// CHECK: = shl i2053 %[[L13]], 7

typedef int int2053_tt __attribute__((__ap_int(2053)));
typedef unsigned int uint2053_tt __attribute__((__ap_int(2053)));

void foo() {
  int2053_tt x2053_s = 0, y2053_s = 0;
  uint2053_tt x2053_u = 0, y2053_u = 0;
  long res = 0;

  // signed (only those that differ from unsigned)
  res = x2053_s % y2053_s;

  res = x2053_s >> 7;

  // unsigned
  res = x2053_u % y2053_u;
  res = x2053_u | y2053_u;
  res = x2053_u & y2053_u;
  res = x2053_u ^ y2053_u;
  res = ~x2053_u;

  res = x2053_u >> 7;
  res = x2053_u << 7;
}