// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ

typedef struct point {
  int x, y, z;
} point;

void test() {
  // CHECK: define dso_local void @test()
  char *data[200];
  // PTR: %data = alloca [200 x i8*], align 16, !intel_dtrans_type ![[ARRAY:[0-9]+]]
  // OPQ: %data = alloca [200 x ptr], align 16, !intel_dtrans_type ![[ARRAY:[0-9]+]]

  // PTR: %local_ptr = alloca i32*, align 8, !intel_dtrans_type ![[INT_PTR:[0-9]+]]
  // OPQ: %local_ptr = alloca ptr, align 8, !intel_dtrans_type ![[INT_PTR:[0-9]+]]
  int *local_ptr;

  struct point local_struct; // This one doesn't need the metadata because
                             //type will be obvious even with opaque ponters

  struct point *local_struct_ptr;
  // PTR: %local_struct_ptr = alloca %struct._ZTS5point.point*, align 8, !intel_dtrans_type ![[POINT_PTR:[0-9]+]]
  // OPQ: %local_struct_ptr = alloca ptr, align 8, !intel_dtrans_type ![[POINT_PTR:[0-9]+]]
  struct point **local_array_ptrptr[100];
  // PTR: %local_array_ptrptr = alloca [100 x %struct._ZTS5point.point**], align 16, !intel_dtrans_type ![[PTRPTR_ARRAY:[0-9]+]]
  // OPQ: %local_array_ptrptr = alloca [100 x ptr], align 16, !intel_dtrans_type ![[PTRPTR_ARRAY:[0-9]+]]
}

int main() {
  test();
  return 0;
}

// CHECK: !intel.dtrans.types = !{![[POINTS:[0-9]+]]}
// CHECK: ![[POINTS]] = !{!"S", %struct._ZTS5point.point zeroinitializer, i32 3, ![[INT:[0-9]+]], ![[INT]], ![[INT]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[ARRAY]] = !{!"A", i32 200, ![[CHAR_PTR:[0-9]+]]}
// CHECK: ![[CHAR_PTR]] = !{i8 0, i32 1}
// CHECK: ![[INT_PTR]] = !{i32 0, i32 1}
// CHECK: ![[POINT_PTR]] = !{%struct._ZTS5point.point zeroinitializer, i32 1}
// CHECK: ![[PTRPTR_ARRAY]] = !{!"A", i32 100, ![[POINT_PTRPTR:[0-9]+]]
// CHECK: ![[POINT_PTRPTR]] = !{%struct._ZTS5point.point zeroinitializer, i32 2}
