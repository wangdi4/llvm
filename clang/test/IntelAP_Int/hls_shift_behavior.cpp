// RUN: %clang -cc1 -O3 -disable-llvm-passes -fhls %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -fsycl-is-device %s -triple spir64-unknown-linux-sycldevice -emit-llvm -o - | FileCheck %s

// CHECK: %[[L0:[0-9]+]] = load i4, i4* %x4_u, align 1
// CHECK: %[[L1:[0-9]+]] = load i32, i32* %shift_amount.addr, align 4
// CHECK: %[[T0:[a-zA-Z0-9]+]] = trunc i32 %[[L1]] to i4
// CHECK: = shl i4 %[[L0]], %[[T0]]

// CHECK: %[[L2:[0-9]+]] = load i5, i5* %x5_u, align 1
// CHECK: %[[L3:[0-9]+]] = load i32, i32* %shift_amount.addr, align 4
// CHECK: %[[T1:[a-zA-Z0-9]+]] = trunc i32 %[[L3]] to i5
// CHECK: = shl i5 %[[L2]], %[[T1]]

// CHECK: %[[L4:[0-9]+]] = load i46, i46* %x46_u, align 8
// CHECK: = shl i46 %[[L4]], 47
// CHECK: %[[L5:[0-9]+]] = load i46, i46* %x46_u, align 8
// CHECK: = shl i46 %[[L5]], -42
// CHECK: %[[L6:[0-9]+]] = load i46, i46* %x46_u, align 8
// CHECK: = shl i46 %[[L6]], -47

// CHECK: %[[L7:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: = shl i43 %[[L7]], 47
// CHECK: %[[L8:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: = shl i43 %[[L8]], -42
// CHECK: %[[L9:[0-9]+]] = load i43, i43* %x43_u, align 8
// CHECK: = shl i43 %[[L9]], -47

typedef unsigned int uint46_tt __attribute__((__ap_int(46)));
typedef unsigned int uint43_tt __attribute__((__ap_int(43)));
typedef unsigned int uint5_tt __attribute__((__ap_int(5)));
typedef unsigned int uint4_tt __attribute__((__ap_int(4)));

#ifdef SYCL_EXTERNAL
SYCL_EXTERNAL
#endif
void foo(int shift_amount) {
  uint4_tt x4_u = 0;
  uint5_tt x5_u = 0;
  uint46_tt x46_u = 0;
  uint43_tt x43_u = 0;
  long res = 0;

  res = x4_u << (uint4_tt) shift_amount;

  res = x5_u << (uint5_tt) shift_amount;

  res = x46_u << 47;
  res = x46_u << -42;
  res = x46_u << -47;

  res = x43_u << 47;
  res = x43_u << -42;
  res = x43_u << -47;
}
