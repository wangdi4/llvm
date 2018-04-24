// RUN: %clang -cc1 -O3 -disable-llvm-passes %s -emit-llvm -o %t
// RUN: FileCheck %s --implicit-check-not "icmp {{.*}} i32" < %t
// CHECK: %[[L0:[0-9]+]] = load i43, i43* %x43_s, align 8
// CHECK: %[[S00:[a-zA-Z0-9]+]] = icmp sgt i43 %[[L0]], 4
// CHECK: %[[S01:[a-zA-Z0-9]+]] = zext i1 %[[S00]] to i8
// CHECK: store i8 %[[S01]], i8* %b, align 1
// CHECK: %[[L1:[0-9]+]] = load i43, i43* %x43_s, align 8
// CHECK: %[[S10:[a-zA-Z0-9]+]] = icmp slt i43 %[[L1]], 4
// CHECK: %[[S11:[a-zA-Z0-9]+]] = zext i1 %[[S10]] to i8
// CHECK: store i8 %[[S11]], i8* %b, align 1
// CHECK: %[[L2:[0-9]+]] = load i43, i43* %x43_s, align 8
// CHECK: %[[S20:[a-zA-Z0-9]+]] = icmp sge i43 %[[L2]], 4
// CHECK: %[[S21:[a-zA-Z0-9]+]] = zext i1 %[[S20]] to i8
// CHECK: store i8 %[[S21]], i8* %b, align 1
// CHECK: %[[L3:[0-9]+]] = load i43, i43* %x43_s, align 8
// CHECK: %[[S30:[a-zA-Z0-9]+]] = icmp sle i43 %[[L3]], 4
// CHECK: %[[S31:[a-zA-Z0-9]+]] = zext i1 %[[S30]] to i8
// CHECK: store i8 %[[S31]], i8* %b, align 1

// CHECK: %[[L4:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U00:[a-zA-Z0-9]+]] = icmp eq i43 %[[L4]], 4
// CHECK: %[[U01:[a-zA-Z0-9]+]] = zext i1 %[[U00]] to i8
// CHECK: store i8 %[[U01]], i8* %b, align 1
// CHECK: %[[L5:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U10:[a-zA-Z0-9]+]] = icmp ne i43 %[[L5]], 4
// CHECK: %[[U11:[a-zA-Z0-9]+]] = zext i1 %[[U10]] to i8
// CHECK: store i8 %[[U11]], i8* %b, align 1
// CHECK: %[[L6:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U20:[a-zA-Z0-9]+]] = icmp ugt i43 %[[L6]], 4
// CHECK: %[[U21:[a-zA-Z0-9]+]] = zext i1 %[[U20]] to i8
// CHECK: store i8 %[[U21]], i8* %b, align 1
// CHECK: %[[L7:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U30:[a-zA-Z0-9]+]] = icmp ult i43 %[[L7]], 4
// CHECK: %[[U31:[a-zA-Z0-9]+]] = zext i1 %[[U30]] to i8
// CHECK: store i8 %[[U31]], i8* %b, align 1
// CHECK: %[[L8:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U40:[a-zA-Z0-9]+]] = icmp uge i43 %[[L8]], 4
// CHECK: %[[U41:[a-zA-Z0-9]+]] = zext i1 %[[U40]] to i8
// CHECK: store i8 %[[U41]], i8* %b, align 1
// CHECK: %[[L9:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: %[[U50:[a-zA-Z0-9]+]] = icmp ule i43 %[[L9]], 4
// CHECK: %[[U51:[a-zA-Z0-9]+]] = zext i1 %[[U50]] to i8
// CHECK: store i8 %[[U51]], i8* %b, align 1

#include "../ihc_apint.h"

kernel void foo() {
  int43_tt x43_s = 0;
  int4_tt x4_s = 0;

  uint43_tt x43_u = 0;
  uint4_tt x4_u = 0;

  bool b;

  // signed (only those that differ from unsigned)
  b = (x43_s > 4);
  b = (x4_s > (uint4_tt) 4);
  b = (x43_s < 4);
  b = (x4_s < (uint4_tt) 4);
  b = (x43_s >= 4);
  b = (x4_s >= (uint4_tt) 4);
  b = (x43_s <= 4);
  b = (x4_s <= (uint4_tt) 4);

  // unsigned
  b = (x43_u == 4);
  b = (x4_u == (uint4_tt) 4);
  b = (x43_u != 4);
  b = (x4_u != (uint4_tt) 4);
  b = (x43_u > 4);
  b = (x4_u > (uint4_tt) 4);
  b = (x43_u < 4);
  b = (x4_u < (uint4_tt) 4);
  b = (x43_u >= 4);
  b = (x4_u >= (uint4_tt) 4);
  b = (x43_u <= 4);
  b = (x4_u <= (uint4_tt) 4);
}
