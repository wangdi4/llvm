// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

typedef struct point {
  int x, y, z;
} point;

void test() {
  // CHECK: define dso_local void @test()
  char *data[200];
  // CHECK: %data = alloca [200 x i8*], align 16, !intel_dtrans_type ![[ARRAY:[0-9]+]]

  // CHECK: %local_ptr = alloca i32*, align 8, !intel_dtrans_type ![[INT_PTR:[0-9]+]]
  int *local_ptr;

  struct point local_struct; // This one doesn't need the metadata because
                             //type will be obvious even with opaque ponters

  struct point *local_struct_ptr;
  // CHECK: %local_struct_ptr = alloca %struct._ZTS5point.point*, align 8, !intel_dtrans_type ![[POINT_PTR:[0-9]+]]
  struct point **local_array_ptrptr[100];
  // CHECK: %local_array_ptrptr = alloca [100 x %struct._ZTS5point.point**], align 16, !intel_dtrans_type ![[PTRPTR_ARRAY:[0-9]+]]
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
