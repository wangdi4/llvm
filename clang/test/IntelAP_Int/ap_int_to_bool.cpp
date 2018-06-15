// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-unknown-linux-gnu -fhls %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-pc-win32 -fhls %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-unknown-intelfpga -fhls %s -emit-llvm -o - | FileCheck %s
// Ensure that we can cast an ap_int to bool.

typedef int int65_tt __attribute__((__ap_int(65)));
typedef unsigned int uint65_tt __attribute__((__ap_int(65)));

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
