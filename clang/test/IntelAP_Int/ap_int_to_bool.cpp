// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-unknown-linux-gnu -fhls %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-pc-win32 -fhls %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-unknown-intelfpga -fhls %s -emit-llvm -o - | FileCheck %s

// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-linux-sycldevice -fsycl-is-device %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-windows-sycldevice -fsycl-is-device %s -emit-llvm -o - | FileCheck %s
// Ensure that we can cast an ap_int to bool.

typedef int int65_tt __attribute__((__ap_int(65)));
typedef unsigned int uint65_tt __attribute__((__ap_int(65)));

#ifndef SYCL_EXTERNAL
#define SYCL_EXTERNAL
#endif

SYCL_EXTERNAL
bool test_ap_int_to_bool()
{
  // CHECK: define
  // CHECK: %[[AP_INT:[a-zA-Z0-9_]+]] = alloca i65
  // CHECK: %[[B:[a-zA-Z0-9_]+]] = alloca i8
  // CHECK: %[[FIFTH_BIT:[a-zA-Z0-9_]+]] = alloca i8
  int65_tt an_ap_int = 10;
  bool b = an_ap_int;
  // CHECK: %[[LOAD:[a-zA-Z0-9_]+]] = load i65, i65* %[[AP_INT]]
  // CHECK: %[[TO_BOOL:[a-zA-Z0-9_]+]] = icmp ne i65 %[[LOAD]], 0
  // CHECK: %[[FROM_BOOL:[a-zA-Z0-9_]+]] = zext i1 %[[TO_BOOL]] to i8
  // CHECK: store i8 %[[FROM_BOOL]], i8* %[[B]]

  bool the_5th_bit = (an_ap_int >> 5) & 1;

  // CHECK: %[[LOAD_1:[a-zA-Z0-9_]+]] = load i65, i65* %[[AP_INT]]
  // CHECK: %[[SHR:[a-zA-Z0-9_]+]] = ashr i65 %[[LOAD_1]]
  // CHECK: %[[AND:[a-zA-Z0-9_]+]] = and i65 %[[SHR]]
  // CHECK: %[[TO_BOOL_1:[a-zA-Z0-9_]+]] = icmp ne i65 %[[AND]], 0
  // CHECK: %[[FROM_BOOL_2:[a-zA-Z0-9_]+]] = zext i1 %[[TO_BOOL_1]] to i8
  // CHECK: store i8 %[[FROM_BOOL_2]], i8* %[[FIFTH_BIT]]

  return an_ap_int;
  // CHECK: %[[RET_CONV:[a-zA-Z0-9_]+]] = load i65, i65* %[[AP_INT]]
  // CHECK: %[[TO_BOOL_3:[a-zA-Z0-9_]+]] = icmp ne i65 %[[RET_CONV]], 0
  // CHECK: ret i1 %[[TO_BOOL_3]]
}

SYCL_EXTERNAL
bool test_ap_uint_to_bool() {
  // CHECK: define
  // CHECK: %[[AP_INT:[a-zA-Z0-9_]+]] = alloca i65
  // CHECK: %[[B:[a-zA-Z0-9_]+]] = alloca i8
  // CHECK: %[[FIFTH_BIT:[a-zA-Z0-9_]+]] = alloca i8
  uint65_tt an_ap_uint = 10;
  bool b = an_ap_uint;
  // CHECK: %[[LOAD:[a-zA-Z0-9_]+]] = load i65, i65* %[[AP_INT]]
  // CHECK: %[[TO_BOOL:[a-zA-Z0-9_]+]] = icmp ne i65 %[[LOAD]], 0
  // CHECK: %[[FROM_BOOL:[a-zA-Z0-9_]+]] = zext i1 %[[TO_BOOL]] to i8
  // CHECK: store i8 %[[FROM_BOOL]], i8* %[[B]]

  bool the_5th_bit = (an_ap_uint >> 5) & 1;

  // CHECK: %[[LOAD_1:[a-zA-Z0-9_]+]] = load i65, i65* %[[AP_INT]]
  // CHECK: %[[SHR:[a-zA-Z0-9_]+]] = lshr i65 %[[LOAD_1]]
  // CHECK: %[[AND:[a-zA-Z0-9_]+]] = and i65 %[[SHR]]
  // CHECK: %[[TO_BOOL_1:[a-zA-Z0-9_]+]] = icmp ne i65 %[[AND]], 0
  // CHECK: %[[FROM_BOOL_2:[a-zA-Z0-9_]+]] = zext i1 %[[TO_BOOL_1]] to i8
  // CHECK: store i8 %[[FROM_BOOL_2]], i8* %[[FIFTH_BIT]]

  return an_ap_uint;
  // CHECK: %[[RET_CONV:[a-zA-Z0-9_]+]] = load i65, i65* %[[AP_INT]]
  // CHECK: %[[TO_BOOL_3:[a-zA-Z0-9_]+]] = icmp ne i65 %[[RET_CONV]], 0
  // CHECK: ret i1 %[[TO_BOOL_3]]
}

template <unsigned int Bits>
using ap_uint = unsigned int __attribute((__ap_int(Bits)));

SYCL_EXTERNAL
void test_ap_int_conversions() {
  ap_uint<5> s(0);
  // CHECK: store i5 0, i5* [[S:%.+]]
  auto t1 = s + true;
  // CHECK: [[S_VAL1:%.+]] = load i5, i5* [[S]]
  // CHECK: [[ADD1:%.+]] = add i5 [[S_VAL1]], 1
  auto t2 = true + s;
  // CHECK: [[S_VAL2:%.+]] = load i5, i5* [[S]]
  // CHECK: [[ADD2:%.+]] = add i5 1, [[S_VAL2]]
  s += true;
  // CHECK: [[S_VAL3:%.+]] = load i5, i5* [[S]]
  // CHECK: [[ADD3:%.+]] = add i5 [[S_VAL3]], 1
  // CHECK: store i5 [[ADD3]], i5* [[S]]

  bool b = true;
  // CHECK: store i8 1, i8* [[B:%.+]]
  b += s;
  // CHECK: [[S_VAL4:%.+]] = load i5, i5* [[S]]
  // CHECK: [[B_VAL:%.+]] = load i8, i8* [[B]]
  // CHECK: [[TO_BOOL1:%.+]] = trunc i8 [[B_VAL]] to i1
  // CHECK: [[CONV1:%.+]] = zext i1 [[TO_BOOL1]] to i5
  // CHECK: [[ADD4:%.+]] = add i5 [[CONV1]], [[S_VAL4]]
  // CHECK: [[TO_BOOL2:%.+]] = icmp ne i5 [[ADD4]]
  // CHECK: [[FROM_BOOL1:%.+]] = zext i1 [[TO_BOOL2]] to i8
  // CHECK: store i8 [[FROM_BOOL1]], i8* [[B]]

  ap_uint<1> s1(0);
  // CHECK: store i1 false, i1* [[S1:%.+]]
  auto t3 = s1 + true;
  // CHECK: [[S1_VAL1:%.+]] = load i1, i1* [[S1]]
  // CHECK: [[ADD5:%.+]] = add i1 [[S1_VAL1]], true
  auto t4 = true + s1;
  // CHECK: [[S1_VAL2:%.+]] = load i1, i1* [[S1]]
  // CHECK: [[ADD6:%.+]] = add i1 true, [[S1_VAL2]]
  s1 += true;
  // CHECK: [[S1_VAL3:%.+]] = load i1, i1* [[S1]]
  // CHECK: [[ADD7:%.+]] = add i1 [[S1_VAL3]], true
  // CHECK: store i1 [[ADD7]], i1* [[S1]]

  bool b1 = true;
  // CHECK: store i8 1, i8* [[B1:%.+]]
  b1 += s1;
  // CHECK: [[S1_VAL4:%.+]] = load i1, i1* [[S1]]
  // CHECK: [[B1_VAL:%.+]] = load i8, i8* [[B1]]
  // CHECK: [[TO_BOOL3:%.+]] = trunc i8 [[B1_VAL]] to i1
  // CHECK: [[ADD8:%.+]] = add i1 [[TO_BOOL3]]
  // CHECK: [[TO_BOOL4:%.+]] = icmp ne i1 [[ADD8]]
  // CHECK: [[FROM_BOOL2:%.+]] = zext i1 [[TO_BOOL4]]
  // CHECK: store i8 [[FROM_BOOL2]], i8* [[B1]]
}

