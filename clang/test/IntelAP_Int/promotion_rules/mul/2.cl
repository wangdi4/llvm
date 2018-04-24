// Ensure the correct promotion rules are followed in operations with 1 ap_[u]int type and 1 base type
// in cases where the ap_[u]int type < sizeof(int)
// RUN: %clang -cc1 -O3 -disable-llvm-passes %s -emit-llvm -o %t
// RUN: FileCheck %s < %t

// CHECK: %[[L0:[0-9]+]] = load i16, i16* %x16_s, align 2
// CHECK: %[[L1:[0-9]+]] = load i16, i16* %short_var, align 2
// CHECK: = mul nsw i16 %[[L0]], %[[L1]]
// CHECK: %[[L2:[0-9]+]] = load i16, i16* %x16_s, align 2
// CHECK: %[[L3:[0-9]+]] = load i16, i16* %u_short_var, align 2
// CHECK: = mul i16 %[[L2]], %[[L3]]
// CHECK: %[[L4:[0-9]+]] = load i16, i16* %x16_u, align 2
// CHECK: %[[L5:[0-9]+]] = load i16, i16* %short_var, align 2
// CHECK: = mul i16 %[[L4]], %[[L5]]
// CHECK: %[[L6:[0-9]+]] = load i16, i16* %x16_u, align 2
// CHECK: %[[L7:[0-9]+]] = load i16, i16* %u_short_var, align 2
// CHECK: = mul i16 %[[L6]], %[[L7]]

// CHECK: %[[L8:[0-9]+]] = load i17, i17* %x17_s, align 4
// CHECK: %[[L9:[0-9]+]] = load i16, i16* %short_var, align 2
// CHECK: %[[M0:[a-zA-Z0-9]+]] = sext i16 %[[L9]] to i17
// CHECK: = mul nsw i17 %[[L8]], %[[M0]]
// CHECK: %[[L10:[0-9]+]] = load i17, i17* %x17_s, align 4
// CHECK: %[[L11:[0-9]+]] = load i16, i16* %u_short_var, align 2
// CHECK: %[[M1:[a-zA-Z0-9]+]] = zext i16 %[[L11]] to i17
// CHECK: = mul nsw i17 %[[L10]], %[[M1]]
// CHECK: %[[L12:[0-9]+]] = load i17, i17* %x17_u, align 4
// CHECK: %[[L13:[0-9]+]] = load i16, i16* %short_var, align 2
// CHECK: %[[M2:[a-zA-Z0-9]+]] = sext i16 %[[L13]] to i17
// CHECK: = mul i17 %[[L12]], %[[M2]]
// CHECK: %[[L14:[0-9]+]] = load i17, i17* %x17_u, align 4
// CHECK: %[[L15:[0-9]+]] = load i16, i16* %u_short_var, align 2
// CHECK: %[[M3:[a-zA-Z0-9]+]] = zext i16 %[[L15]] to i17
// CHECK: = mul i17 %[[L14]], %[[M3]]

// CHECK: %[[L16:[0-9]+]] = load i15, i15* %x15_s, align 2
// CHECK: %[[M4:[a-zA-Z0-9]+]] = sext i15 %[[L16]] to i16
// CHECK: %[[L17:[0-9]+]] = load i16, i16* %short_var, align 2
// CHECK: = mul nsw i16 %[[M4]], %[[L17]]
// CHECK: %[[L18:[0-9]+]] = load i15, i15* %x15_s, align 2
// CHECK: %[[M5:[a-zA-Z0-9]+]] = sext i15 %[[L18]] to i16
// CHECK: %[[L19:[0-9]+]] = load i16, i16* %u_short_var, align 2
// CHECK: = mul i16 %[[M5]], %[[L19]]
// CHECK: %[[L20:[0-9]+]] = load i15, i15* %x15_u, align 2
// CHECK: %[[M6:[a-zA-Z0-9]+]] = zext i15 %[[L20]] to i16
// CHECK: %[[L21:[0-9]+]] = load i16, i16* %short_var, align 2
// CHECK: = mul nsw i16 %[[M6]], %[[L21]]
// CHECK: %[[L22:[0-9]+]] = load i15, i15* %x15_u, align 2
// CHECK: %[[M7:[a-zA-Z0-9]+]] = zext i15 %[[L22]] to i16
// CHECK: %[[L23:[0-9]+]] = load i16, i16* %u_short_var, align 2
// CHECK: = mul i16 %[[M7]], %[[L23]]

#include "../../ihc_apint.h"

kernel void foo() {
  int15_tt x15_s = 0;
  uint15_tt x15_u = 0;

  int16_tt x16_s = 0;
  uint16_tt x16_u = 0;

  int17_tt x17_s = 0;
  uint17_tt x17_u = 0;

  short short_var = 0;
  unsigned short u_short_var = 0;

  long res = 0;

  // 1 ap + 1 base type. < 32 bits
  // same size
  res = x16_s * short_var;
  res = x16_s * u_short_var;
  res = x16_u * short_var;
  res = x16_u * u_short_var;

  // ap > base type
  res = x17_s * short_var;
  res = x17_s * u_short_var;
  res = x17_u * short_var;
  res = x17_u * u_short_var;

  // base type > ap
  res = x15_s * short_var;
  res = x15_s * u_short_var;
  res = x15_u * short_var;
  res = x15_u * u_short_var;
}
