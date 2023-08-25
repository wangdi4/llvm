// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ

struct arc {
  int id;
  long cost;
  struct arc *next;
};

typedef int (*compare_func)(struct arc **, struct arc **);

struct arc A1 = {0, 100};
struct arc A2 = {1, 99};
struct arc *all_arcs[2] = {&A1, &A2};

int arcCostLess(struct arc **arc1, struct arc **arc2) {
  return (*arc2)->cost - (*arc1)->cost;
}

compare_func arc_cmp = &arcCostLess;
int test(void *arcList) {
  struct arc **arc0 = (struct arc **)(arcList);
  struct arc **arc1 = arc0 + 1;

  // This call should get tagged to indicate signature of the indirect call target.
  return arc_cmp(arc0, arc1);
}

// OPQ: @all_arcs = global [2 x ptr] [ptr @A1, ptr @A2], align 16, !intel_dtrans_type ![[ARC_PTR_ARRAY:[0-9]+]]
// OPQ: @arc_cmp = global ptr @arcCostLess, align 8, !intel_dtrans_type ![[FPTR:[0-9]+]]

// OPQ: define dso_local i32 @arcCostLess(ptr noundef "intel_dtrans_func_index"="1" %{{.+}}, ptr noundef "intel_dtrans_func_index"="2" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[COSTLESS_FUNC_MD:[0-9]+]]
// OPQ: %arc1.addr = alloca ptr, align 8, !intel_dtrans_type ![[ARC_PTR_PTR:[0-9]+]]
// OPQ: %arc2.addr = alloca ptr, align 8, !intel_dtrans_type ![[ARC_PTR_PTR]]

// OPQ: define dso_local i32 @test(ptr noundef "intel_dtrans_func_index"="1" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[TEST_FUNC_MD:[0-9]+]]
// OPQ: %arcList.addr = alloca ptr, align 8, !intel_dtrans_type ![[VOID_PTR:[0-9]+]]
// OPQ: %[[ARC0:.+]] = alloca ptr, align 8, !intel_dtrans_type ![[ARC_PTR_PTR]]
// OPQ: %[[ARC1:.+]] = alloca ptr, align 8, !intel_dtrans_type ![[ARC_PTR_PTR]]
// OPQ: %[[FUNC_LOAD:.+]] = load ptr, ptr @arc_cmp
// OPQ: %[[ARC0_LOAD:.+]] = load ptr, ptr %[[ARC0]]
// OPQ: %[[ARC1_LOAD:.+]] = load ptr, ptr %[[ARC1]]
// OPQ: call i32 %[[FUNC_LOAD]](ptr noundef %[[ARC0_LOAD]], ptr noundef %[[ARC1_LOAD]]), !intel_dtrans_type ![[FUNC:[0-9]+]]
// CHECK: !intel.dtrans.types = !{![[ARC:[0-9]+]]}

// CHECK: ![[ARC_PTR_ARRAY]] = !{!"A", i32 2, ![[ARC_PTR:[0-9]+]]}
// CHECK: ![[ARC_PTR]] = !{%struct._ZTS3arc.arc zeroinitializer, i32 1}
// CHECK: ![[FPTR]] = !{![[FUNC]], i32 1}
// CHECK: ![[FUNC]] = !{!"F", i1 false, i32 2, ![[INT:[0-9]+]], ![[ARC_PTR_PTR]], ![[ARC_PTR_PTR]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[ARC]] = !{!"S", %struct._ZTS3arc.arc zeroinitializer, i32 3, ![[INT]], ![[LONG:[0-9]+]], ![[ARC_PTR]]}
// CHECK: ![[LONG]] = !{i64 0, i32 0}

// CHECK: ![[COSTLESS_FUNC_MD]] = distinct !{![[ARC_PTR_PTR]], ![[ARC_PTR_PTR]]}
// CHECK: ![[TEST_FUNC_MD]] = distinct !{![[VOID_PTR]]}
// CHECK: ![[VOID_PTR]] = !{i8 0, i32 1}
