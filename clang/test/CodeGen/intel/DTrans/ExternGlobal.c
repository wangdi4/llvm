// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
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

// PTR: @basket = global %struct._ZTS6basket.basket* null, align 8, !intel_dtrans_type ![[BASKET_PTR:[0-9]+]]
// OPQ: @basket = global ptr null, align 8, !intel_dtrans_type ![[BASKET_PTR:[0-9]+]]
// PTR: @perm_p = external global %struct._ZTS6basket.basket**, align 8, !intel_dtrans_type ![[BASKET_PTR_PTR:[0-9]+]]
// OPQ: @perm_p = external global ptr, align 8, !intel_dtrans_type ![[BASKET_PTR_PTR:[0-9]+]]

int main() {
  perm_p = &basket;
  return 0;
}

// CHECK: !intel.dtrans.types = !{![[BASKET:[0-9]+]], ![[ARC:[0-9]+]]}
// CHECK: ![[BASKET_PTR]] = !{%struct._ZTS6basket.basket zeroinitializer, i32 1}
// CHECK: ![[BASKET_PTR_PTR]] = !{%struct._ZTS6basket.basket zeroinitializer, i32 2}
// CHECK: ![[BASKET]] = !{!"S", %struct._ZTS6basket.basket zeroinitializer, i32 1, ![[ARC_PTR:[0-9]+]]}
// CHECK: ![[ARC_PTR]] = !{%struct._ZTS3arc.arc zeroinitializer, i32 1}
// CHECK: ![[ARC]] = !{!"S", %struct._ZTS3arc.arc zeroinitializer, i32 3, ![[INT:[0-9]+]], ![[LONG:[0-9]+]], ![[ARC_PTR]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[LONG]] = !{i64 0, i32 0}
