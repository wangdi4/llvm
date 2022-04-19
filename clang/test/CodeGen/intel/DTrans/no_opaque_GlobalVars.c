// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ

struct unknown;

struct arc {
  int id;
  long cost;
  struct arc *next;
};

struct basket {
  struct arc *arc;
};

typedef int (*compare_func)(struct arc *, struct arc *);

struct basket *basket;
struct basket ***perm_p;
int *basket_sizes;

struct arc a_arc; // No need to tag this one, getValueType() will return actual type.
struct arc *all_arc_ptrs[256];
struct arc *const ptr_arc = &a_arc; // const definition, instead of a global.

int arcless(struct arc *, struct arc *);
compare_func arc_cmp = &arcless;

int main() {
  return 0;
}

// PTR: @ptr_arc = constant %struct._ZTS3arc.arc* @a_arc, align 8, !intel_dtrans_type ![[ARC_PTR:[0-9]+]]
// OPQ: @ptr_arc = constant ptr @a_arc, align 8, !intel_dtrans_type ![[ARC_PTR:[0-9]+]]
// PTR: @arc_cmp = global i32 (%struct._ZTS3arc.arc*, %struct._ZTS3arc.arc*)* @arcless, align 8, !intel_dtrans_type ![[FPTR:[0-9]+]]
// OPQ: @arc_cmp = global ptr @arcless, align 8, !intel_dtrans_type ![[FPTR:[0-9]+]]
// PTR: @basket = global %struct._ZTS6basket.basket* null, align 8, !intel_dtrans_type ![[BASKET_PTR:[0-9]+]]
// OPQ: @basket = global ptr null, align 8, !intel_dtrans_type ![[BASKET_PTR:[0-9]+]]
// PTR: @perm_p = global %struct._ZTS6basket.basket*** null, align 8, !intel_dtrans_type ![[BASKET_PTRPTRPTR:[0-9]+]]
// OPQ: @perm_p = global ptr null, align 8, !intel_dtrans_type ![[BASKET_PTRPTRPTR:[0-9]+]]
// PTR: @basket_sizes = global i32* null, align 8, !intel_dtrans_type ![[INT_PTR:[0-9]+]]
// OPQ: @basket_sizes = global ptr null, align 8, !intel_dtrans_type ![[INT_PTR:[0-9]+]]
// PTR: @all_arc_ptrs = global [256 x %struct._ZTS3arc.arc*] zeroinitializer, align 16, !intel_dtrans_type ![[ARC_PTR_ARRAY:[0-9]+]]
// OPQ: @all_arc_ptrs = global [256 x ptr] zeroinitializer, align 16, !intel_dtrans_type ![[ARC_PTR_ARRAY:[0-9]+]]

// PTR: declare !intel.dtrans.func.type ![[ARCLESS_MD:[0-9]+]] i32 @arcless(%struct._ZTS3arc.arc* noundef "intel_dtrans_func_index"="1", %struct._ZTS3arc.arc* noundef "intel_dtrans_func_index"="2")
// OPQ: declare !intel.dtrans.func.type ![[ARCLESS_MD:[0-9]+]] i32 @arcless(ptr noundef "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2")
// CHECK: !intel.dtrans.types = !{![[ARC:[0-9]+]], ![[BASKET:[0-9]+]]}

// CHECK: ![[ARC_PTR]] = !{%struct._ZTS3arc.arc zeroinitializer, i32 1}
// CHECK: ![[FPTR]] = !{![[FUNC:[0-9]+]], i32 1}
// CHECK: ![[FUNC]] = !{!"F", i1 false, i32 2, ![[INT:[0-9]+]], ![[ARC_PTR]], ![[ARC_PTR]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[BASKET_PTR]] = !{%struct._ZTS6basket.basket zeroinitializer, i32 1}
// CHECK: ![[BASKET_PTRPTRPTR]] = !{%struct._ZTS6basket.basket zeroinitializer, i32 3}
// CHECK: ![[INT_PTR]] = !{i32 0, i32 1}
// CHECK: ![[ARC_PTR_ARRAY]] = !{!"A", i32 256, ![[ARC_PTR]]}
// CHECK: ![[ARC]] = !{!"S", %struct._ZTS3arc.arc zeroinitializer, i32 3, ![[INT]], ![[LONG:[0-9]+]], ![[ARC_PTR]]}
// CHECK: ![[LONG]] = !{i64 0, i32 0}
// CHECK: ![[BASKET]] = !{!"S", %struct._ZTS6basket.basket zeroinitializer, i32 1, ![[ARC_PTR]]}
// CHECK: ![[ARCLESS_MD]] = distinct !{![[ARC_PTR]], ![[ARC_PTR]]}
