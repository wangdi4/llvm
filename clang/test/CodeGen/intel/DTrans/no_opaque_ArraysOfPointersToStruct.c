// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s

struct test_simple {
  int id;
  long cost;
  short ident;
  double flow;
  long org_cost;
};

struct test_array {
  // Array of structures
  struct test_simple inst[100];

  // Array of pointers to structures
  struct test_simple *ptr[100];
};

int main() {
  struct test_array instance;

  instance.inst[0].id = 0;
  instance.ptr[0] = (void *)0;

  return 0;
}

// CHECK: !intel.dtrans.types = !{![[HASARRAYS:[0-9]+]], ![[SIMPLE:[0-9]+]]}
// CHECK: ![[HASARRAYS]] = !{!"S", %struct._ZTS10test_array.test_array zeroinitializer, i32 2, ![[STRUCT_ARRAY:[0-9]+]], ![[STRUCT_PTR_ARRAY:[0-9]+]]}
// CHECK: ![[STRUCT_ARRAY]] = !{!"A", i32 100, ![[STRUCT:[0-9]+]]}
// CHECK: ![[STRUCT]] = !{%struct._ZTS11test_simple.test_simple zeroinitializer, i32 0}
// CHECK: ![[STRUCT_PTR_ARRAY]] = !{!"A", i32 100, ![[STRUCT_PTR:[0-9]+]]}
// CHECK: ![[STRUCT_PTR]] = !{%struct._ZTS11test_simple.test_simple zeroinitializer, i32 1}

// CHECK: ![[SIMPLE]] = !{!"S", %struct._ZTS11test_simple.test_simple zeroinitializer, i32 5, ![[INT:[0-9]+]], ![[LONG:[0-9]+]], ![[SHORT:[0-9]+]], ![[DOUBLE:[0-9]+]], ![[LONG]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[LONG]] = !{i64 0, i32 0}
// CHECK: ![[SHORT]] = !{i16 0, i32 0}
// CHECK: ![[DOUBLE]] = !{double 0.0{{0+}}e+00, i32 0}
