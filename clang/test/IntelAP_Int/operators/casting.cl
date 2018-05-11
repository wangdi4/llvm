// RUN: %clang -cc1 -O3 -disable-llvm-passes %s -emit-llvm -o %t
// RUN: FileCheck %s < %t
// CHECK: %[[L0:[0-9]+]] = load i5, i5* %x5_s, align 1
// CHECK: %[[S0:[a-zA-Z0-9]+]] = sext i5 %[[L0]] to i8
// CHECK: store i8 %[[S0]], i8* %c, align 1
// CHECK: %[[L1:[0-9]+]] = load i5, i5* %x5_s, align 1
// CHECK: %[[S1:[a-zA-Z0-9]+]] = sext i5 %[[L1]] to i16
// CHECK: store i16 %[[S1]], i16* %s, align 2
// CHECK: %[[L2:[0-9]+]] = load i5, i5* %x5_s, align 1
// CHECK: %[[S2:[a-zA-Z0-9]+]] = sext i5 %[[L2]] to i32
// CHECK: store i32 %[[S2]], i32* %i, align 4
// CHECK: %[[L3:[0-9]+]] = load i5, i5* %x5_s, align 1
// CHECK: %[[S3:[a-zA-Z0-9]+]] = sext i5 %[[L3]] to i64
// CHECK: store i64 %[[S3]], i64* %l, align 8

// CHECK: %[[L5:[0-9]+]] = load i5, i5* %x5_u, align 1
// CHECK: %[[U00:[a-zA-Z0-9]+]] = icmp ne i5 %[[L5]], 0
// CHECK: %[[U01:[a-zA-Z0-9]+]] = zext i1 %[[U00]] to i8
// CHECK: store i8 %[[U01]], i8* %b, align 1
// CHECK: %[[L6:[0-9]+]] = load i5, i5* %x5_u, align 1
// CHECK: %[[U0:[a-zA-Z0-9]+]] = zext i5 %[[L6]] to i8
// CHECK: store i8 %[[U0]], i8* %c, align 1
// CHECK: %[[L7:[0-9]+]] = load i5, i5* %x5_u, align 1
// CHECK: %[[U1:[a-zA-Z0-9]+]] = zext i5 %[[L7]] to i16
// CHECK: store i16 %[[U1]], i16* %s, align 2
// CHECK: %[[L8:[0-9]+]] = load i5, i5* %x5_u, align 1
// CHECK: %[[U2:[a-zA-Z0-9]+]] = zext i5 %[[L8]] to i32
// CHECK: store i32 %[[U2]], i32* %i, align 4
// CHECK: %[[L9:[0-9]+]] = load i5, i5* %x5_u, align 1
// CHECK: %[[U3:[a-zA-Z0-9]+]] = zext i5 %[[L9]] to i64
// CHECK: store i64 %[[U3]], i64* %l, align 8

#include "../ihc_apint.h"

kernel void foo() {
  int5_tt x5_s = 0;
  uint5_tt x5_u = 0;
  bool b = 0;
  char c = 0;
  short s = 0;
  int i = 0;
  long l = 0;

  // signed (only those that differ from unsigned)
  c = (char)x5_s;       // ver with unsigned base type has same IR
  s = (short)x5_s;      // ver with unsigned base type has same IR
  i = (int)x5_s;        // ver with unsigned base type has same IR
  l = (long)x5_s;       // ver with unsigned base type has same IR

  // unsigned
  b = (bool)x5_u;
  c = (char)x5_u;       // ver with unsigned base type has same IR
  s = (short)x5_u;      // ver with unsigned base type has same IR
  i = (int)x5_u;        // ver with unsigned base type has same IR
  l = (long)x5_u;       // ver with unsigned base type has same IR
}
