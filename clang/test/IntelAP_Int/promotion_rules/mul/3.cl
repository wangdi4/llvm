// Ensure the correct promotion rules are followed in operations with 1 ap_[u]int type and 1 base type
// in cases where the ap_[u]int type >= sizeof(int)
// RUN: %clang -cc1 -O3 -disable-llvm-passes %s -emit-llvm -o %t
// RUN: FileCheck %s < %t
// CHECK: %[[L0:[0-9]+]] = load i32, i32* %x32_s, align 4
// CHECK: %[[L1:[0-9]+]] = load i32, i32* %int_var, align 4
// CHECK: = mul nsw i32 %[[L0]], %[[L1]]
// CHECK: %[[L2:[0-9]+]] = load i32, i32* %x32_s, align 4
// CHECK: %[[L3:[0-9]+]] = load i32, i32* %u_int_var, align 4
// CHECK: = mul i32 %[[L2]], %[[L3]]
// CHECK: %[[L3:[0-9]+]] = load i32, i32* %x32_u, align 4
// CHECK: %[[L4:[0-9]+]] = load i32, i32* %int_var, align 4
// CHECK: = mul i32 %[[L3]], %[[L4]]
// CHECK: %[[L5:[0-9]+]] = load i32, i32* %x32_u, align 4
// CHECK: %[[L6:[0-9]+]] = load i32, i32* %u_int_var, align 4
// CHECK: = mul i32 %[[L5]], %[[L6]]

// CHECK: %[[L7:[0-9]+]] = load i33, i33* %x33_s, align 8
// CHECK: %[[L8:[0-9]+]] = load i32, i32* %int_var, align 4
// CHECK: %[[M0:[a-zA-Z0-9]+]] = sext i32 %[[L8]] to i33
// CHECK: = mul nsw i33 %[[L7]], %[[M0]]
// CHECK: %[[L9:[0-9]+]] = load i33, i33* %x33_s, align 8
// CHECK: %[[L10:[0-9]+]] = load i32, i32* %u_int_var, align 4
// CHECK: %[[M1:[a-zA-Z0-9]+]] = zext i32 %[[L10]] to i33
// CHECK: = mul nsw i33 %[[L9]], %[[M1]]
// CHECK: %[[L11:[0-9]+]] = load i33, i33* %x33_u, align 8
// CHECK: %[[L12:[0-9]+]] = load i32, i32* %int_var, align 4
// CHECK: %[[M2:[a-zA-Z0-9]+]] = sext i32 %[[L12]] to i33
// CHECK: = mul i33 %[[L11]], %[[M2]]
// CHECK: %[[L13:[0-9]+]] = load i33, i33* %x33_u, align 8
// CHECK: %[[L14:[0-9]+]] = load i32, i32* %u_int_var, align 4
// CHECK: %[[M3:[a-zA-Z0-9]+]] = zext i32 %[[L14]] to i33
// CHECK: = mul i33 %[[L13]], %[[M3]]

// CHECK: %[[L15:[0-9]+]] = load i33, i33* %x33_s, align 8
// CHECK: %[[M4:[a-zA-Z0-9]+]] = sext i33 %[[L15]] to i64
// CHECK: %[[L16:[0-9]+]] = load i64, i64* %l_var, align 8
// CHECK: = mul nsw i64 %[[M4]], %[[L16]]
// CHECK: %[[L17:[0-9]+]] = load i33, i33* %x33_s, align 8
// CHECK: %[[M5:[a-zA-Z0-9]+]] = sext i33 %[[L17]] to i64
// CHECK: %[[L18:[0-9]+]] = load i64, i64* %u_l_var, align 8
// CHECK: = mul i64 %[[M5]], %[[L18]]
// CHECK: %[[L19:[0-9]+]] = load i33, i33* %x33_u, align 8
// CHECK: %[[M6:[a-zA-Z0-9]+]] = zext i33 %[[L19]] to i64
// CHECK: %[[L20:[0-9]+]] = load i64, i64* %l_var, align 8
// CHECK: = mul nsw i64 %[[M6]], %[[L20]]
// CHECK: %[[L21:[0-9]+]] = load i33, i33* %x33_u, align 8
// CHECK: %[[M7:[a-zA-Z0-9]+]] = zext i33 %[[L21]] to i64
// CHECK: %[[L22:[0-9]+]] = load i64, i64* %u_l_var, align 8
// CHECK: = mul i64 %[[M7]], %[[L22]]

#include "../../ihc_apint.h"

kernel void foo() {
  int32_tt x32_s = 0;
  uint32_tt x32_u = 0;

  int33_tt x33_s = 0;
  uint33_tt x33_u = 0;

  // short short_var = 0; unsigned short u_short_var = 0;
  int int_var = 0;
  unsigned int u_int_var = 0;
  long l_var = 0;
  unsigned long u_l_var = 0;

  long res = 0;

  // 1 ap + 1 base type. >= 32 bits
  // same size
  res = x32_s * int_var;
  res = x32_s * u_int_var;
  res = x32_u * int_var;
  res = x32_u * u_int_var;

  // ap > base type
  res = x33_s * int_var;
  res = x33_s * u_int_var;
  res = x33_u * int_var;
  res = x33_u * u_int_var;

  // base type > ap
  res = x33_s * l_var;
  res = x33_s * u_l_var;
  res = x33_u * l_var;
  res = x33_u * u_l_var;
}
