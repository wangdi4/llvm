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

int main() {
  struct test_simple instance;

  instance.id = 0;
  instance.cost = 0;
  instance.ident = 0;
  instance.flow = 0;
  instance.org_cost = 0;

  return 0;
}

// CHECK: !intel.dtrans.types = !{![[STRUCT_NODE:[0-9]+]]}
// CHECK: ![[STRUCT_NODE]] = !{!"S", %struct._ZTS11test_simple.test_simple zeroinitializer, i32 5, ![[F1:[0-9]+]], ![[F2:[0-9]+]], ![[F3:[0-9]+]], ![[F4:[0-9]+]], ![[F2]]}
// CHECK: ![[F1]] = !{i32 0, i32 0}
// CHECK: ![[F2]] = !{i64 0, i32 0}
// CHECK: ![[F3]] = !{i16 0, i32 0}
// CHECK: ![[F4]] = !{double 0.0{{0+}}e+00, i32 0}
