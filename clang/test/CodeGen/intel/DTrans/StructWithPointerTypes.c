// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s
//
typedef struct test_arc_pointers arc_t;
typedef struct test_arc_pointers *arc_p;

typedef long cost_t;
typedef struct test_ptr2 {
  arc_t *arc;
  cost_t *cost;
  cost_t *abs_cost;
} test_ptr2_t;

struct test_arc_pointers {
  int id;
  long cost;
  short ident;
  double flow;
  long org_cost;
  arc_p nextout, nextin;
};

int main() {
  struct test_arc_pointers instance;
  test_ptr2_t instance2;

  instance.id = 0;
  instance.cost = 0;
  instance.ident = 0;
  instance.flow = 0;
  instance.org_cost = 0;
  instance.nextout = (void *)0;
  instance.nextin = (void *)0;

  instance2.arc = (void *)0;
  instance2.cost = (void *)0;
  instance2.abs_cost = (void *)0;

  return 0;
}

// CHECK: intel.dtrans.types = !{![[TAP:[0-9]+]], ![[TP2:[0-9]+]]}

// CHECK: ![[TAP]] = !{!"S", %struct._ZTS17test_arc_pointers.test_arc_pointers zeroinitializer, i32 7, ![[INT:[0-9]+]], ![[LONG:[0-9]+]], ![[SHORT:[0-9]+]], ![[DOUBLE:[0-9]+]], ![[LONG]], ![[STRUCT_PTR:[0-9]+]], ![[STRUCT_PTR]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[LONG]] = !{i64 0, i32 0}
// CHECK: ![[SHORT]] = !{i16 0, i32 0}
// CHECK: ![[DOUBLE]] = !{double 0.0{{0+}}e+00, i32 0}
// CHECK: ![[STRUCT_PTR]] = !{%struct._ZTS17test_arc_pointers.test_arc_pointers zeroinitializer, i32 1}

// CHECK: ![[TP2]] = !{!"S", %struct._ZTS9test_ptr2.test_ptr2 zeroinitializer, i32 3, ![[STRUCT_PTR]], ![[LONG_PTR:[0-9]+]], ![[LONG_PTR]]}
// CHECK: ![[LONG_PTR]] = !{i64 0, i32 1}
