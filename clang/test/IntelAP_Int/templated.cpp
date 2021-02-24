// Based on the HLS ac_int header file. Purpose: depict a class with
// 1. member variable whose width depends on the template parameters, and
// 2. method that uses ap_[u]ints whose type depends on the template parameters

// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-windows-pc -fhls %s -emit-llvm -o - | FileCheck %s -check-prefixes=CHECK,HLS
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls %s -emit-llvm -o - | FileCheck %s -check-prefixes=CHECK,HLS

// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-windows-sycldevice -fsycl -fsycl-is-device %s -emit-llvm -o - | FileCheck %s -check-prefixes=CHECK,SYCL
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-linux-sycldevice -fsycl -fsycl-is-device %s -emit-llvm -o - | FileCheck %s -check-prefixes=CHECK,SYCL

// CHECK: %a = alloca i3, align 1
// SYCL: %a.ascast = addrspacecast i3* %a to i3 addrspace(4)*
// CHECK: %b = alloca i3, align 1
// SYCL: %b.ascast = addrspacecast i3* %b to i3 addrspace(4)*

// HLS: %[[U0:[0-9]+]] = load i2, i2* %value{{([0-9]+)?}}, align 1
// SYCL: %[[U0:[0-9]+]] = load i2, i2 addrspace(4)* %value{{([0-9]+)?}}, align 1
// CHECK: %[[U1:[a-zA-Z0-9]+]] = zext i2 %[[U0]] to i3
// HLS: store i3 %[[U1]], i3* %a, align 1
// SYCL: store i3 %[[U1]], i3 addrspace(4)* %a.ascast, align 1

// HLS: %[[U2:[0-9]+]] = load i2, i2* %value{{([0-9]+)?}}, align 1
// SYCL: %[[U2:[0-9]+]] = load i2, i2 addrspace(4)* %value{{([0-9]+)?}}, align 1
// CHECK: %[[U3:[a-zA-Z0-9]+]] = zext i2 %[[U2]] to i3
// HLS: store i3 %[[U3]], i3* %b, align 1
// SYCL: store i3 %[[U3]], i3 addrspace(4)* %b.ascast, align 1

// HLS: %[[U4:[0-9]+]] = load i3, i3* %a, align 1
// SYCL: %[[U4:[0-9]+]] = load i3, i3 addrspace(4)* %a.ascast, align 1
// HLS: %[[U5:[0-9]+]] = load i3, i3* %b, align 1
// SYCL: %[[U5:[0-9]+]] = load i3, i3 addrspace(4)* %b.ascast, align 1
// CHECK: %[[U6:[a-zA-Z0-9]+]] = icmp eq i3 %[[U4]], %[[U5]]
// CHECK: ret i1 %[[U6]]

// CHECK: %[[ONEBIT:[a-zA-Z0-9_]+]] = alloca i1, align 1
// SYCL: %[[ONEBIT_CAST:.+]] = addrspacecast i1* %[[ONEBIT]] to i1 addrspace(4)*
// HLS: store i1 false, i1* %[[ONEBIT]], align 1
// SYCL: store i1 false, i1 addrspace(4)* %[[ONEBIT_CAST]], align 1
// HLS: %[[OB1:[0-9]+]] = load i1, i1* %[[ONEBIT]], align 1
// SYCL: %[[OB1:[0-9]+]] = load i1, i1 addrspace(4)* %[[ONEBIT_CAST]], align 1
// CHECK: %[[OB_CONV:[a-zA-Z0-9]+]] = zext i1 %[[OB1]] to i32
// HLS: store i32 %[[OB_CONV]], i32*
// SYCL: i32 %[[OB_CONV]], i32 addrspace(4)*

#define AC_MAX(a, b) ((a) > (b) ? (a) : (b))
#ifndef SYCL_EXTERNAL
#define SYCL_EXTERNAL
#endif

template <int W, bool S>
struct select_type {};

template <int W>
struct select_type<W, true> {
  using type = int __attribute__((__ap_int(W)));
};

template <int W>
struct select_type<W, false> {
  using type = unsigned int __attribute__((__ap_int(W)));
};

template <int N, bool S>
class iv {
public:
  typedef typename select_type<N, S>::type actype;
  actype value;

  template <int Bits>
  using ap_int = int __attribute__((__ap_int(Bits)));
  template <int N2, bool S2>
  bool equal(const iv<N2, S2> &op2) const {
    enum { Sx = AC_MAX(N, N2) };
    ap_int<Sx + 1> a = (ap_int<Sx + 1>)value;
    ap_int<Sx + 1> b = (ap_int<Sx + 1>)op2.value;
    return (a == b);
  }
};

SYCL_EXTERNAL
bool foo() {
  iv<2, false> x, y;
  x.value = 3;
  y.value = 3;
  return x.equal(y);
}

template <unsigned int Bits>
using ap_uint = unsigned int __attribute__((__ap_int(Bits)));


SYCL_EXTERNAL
void bar() {
  ap_uint<1> unsigned_one_bit = 0;
  int i = unsigned_one_bit;
}
