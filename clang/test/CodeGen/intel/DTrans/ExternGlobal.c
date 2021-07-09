// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
struct arc {
  int id;
  long cost;
  struct arc *next;
};

struct basket {
  struct arc *arc;
};

struct basket *basket;
extern struct basket **perm_p;

// CHECK: @basket = global %struct.basket* null, align 8, !intel_dtrans_type ![[BASKET_PTR:[0-9]+]]
// CHECK: @perm_p = external global %struct.basket**, align 8, !intel_dtrans_type ![[BASKET_PTR_PTR:[0-9]+]]

int main() {
  perm_p = &basket;
  return 0;
}

// CHECK: !intel.dtrans.types = !{![[BASKET:[0-9]+]], ![[ARC:[0-9]+]]}
// CHECK: ![[BASKET_PTR]] = !{%struct.basket zeroinitializer, i32 1}
// CHECK: ![[BASKET_PTR_PTR]] = !{%struct.basket zeroinitializer, i32 2}
// CHECK: ![[BASKET]] = !{!"S", %struct.basket zeroinitializer, i32 1, ![[ARC_PTR:[0-9]+]]}
// CHECK: ![[ARC_PTR]] = !{%struct.arc zeroinitializer, i32 1}
// CHECK: ![[ARC]] = !{!"S", %struct.arc zeroinitializer, i32 3, ![[INT:[0-9]+]], ![[LONG:[0-9]+]], ![[ARC_PTR]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[LONG]] = !{i64 0, i32 0}
