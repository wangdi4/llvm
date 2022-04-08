// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ

struct arc_t;

struct test_data {
  int x, y, z;
};

typedef struct test_ptr2ptr {
  // a pointer to an opaque type
  struct arc_t *arc;

  struct test_ptr2ptr *next;
  struct test_data **data;
  struct test_data d;
} test_ptr2ptr_t;

int main() {
  test_ptr2ptr_t instance3;

  instance3.arc = (void *)0;
  instance3.next = (void *)0;
  instance3.data = (void *)0;

  return 0;
}

// PTR: intel.dtrans.types = !{![[TP2P:[0-9]+]], ![[ARC:[0-9]+]], ![[TD:[0-9]+]]}
// OPQ: intel.dtrans.types = !{![[TP2P:[0-9]+]], ![[TD:[0-9]+]], ![[ARC:[0-9]+]]}

// CHECK: ![[TP2P]] = !{!"S", %struct._ZTS12test_ptr2ptr.test_ptr2ptr zeroinitializer, i32 4, ![[ARC_PTR:[0-9]+]], ![[SELF_PTR:[0-9]+]], ![[TD_PTR_PTR:[0-9]+]], ![[STRUCT:[0-9]+]]}
// CHECK: ![[ARC_PTR]] = !{%struct._ZTS5arc_t.arc_t zeroinitializer, i32 1}
// CHECK: ![[SELF_PTR]] = !{%struct._ZTS12test_ptr2ptr.test_ptr2ptr zeroinitializer, i32 1}
// CHECK: ![[TD_PTR_PTR]] = !{%struct._ZTS9test_data.test_data zeroinitializer, i32 2}
// CHECK: ![[STRUCT]] = !{%struct._ZTS9test_data.test_data zeroinitializer, i32 0}

// CHECK-DAG: ![[ARC]] = !{!"S", %struct._ZTS5arc_t.arc_t zeroinitializer, i32 -1}
// CHECK-DAG: ![[TD]] = !{!"S", %struct._ZTS9test_data.test_data zeroinitializer, i32 3, ![[INT:[0-9]+]], ![[INT]], ![[INT]]}

// CHECK-DAG: ![[INT]] = !{i32 0, i32 0}
