// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

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

// CHECK: @all_arcs = global [2 x %struct.arc*] [%struct.arc* @A1, %struct.arc* @A2], align 16, !intel_dtrans_type ![[ARC_PTR_ARRAY:[0-9]+]]
// CHECK: @arc_cmp = global i32 (%struct.arc**, %struct.arc**)* @arcCostLess, align 8, !intel_dtrans_type ![[FPTR:[0-9]+]]

// CHECK: define dso_local i32 @arcCostLess(%struct.arc** "intel_dtrans_func_index"="1" %{{.+}}, %struct.arc** "intel_dtrans_func_index"="2" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[COSTLESS_FUNC_MD:[0-9]+]]
// CHECK: %arc1.addr = alloca %struct.arc**, align 8, !intel_dtrans_type ![[ARC_PTR_PTR:[0-9]+]]
// CHECK: %arc2.addr = alloca %struct.arc**, align 8, !intel_dtrans_type ![[ARC_PTR_PTR]]

// CHECK: define dso_local i32 @test(i8* "intel_dtrans_func_index"="1" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[TEST_FUNC_MD:[0-9]+]]
// CHECK: %arcList.addr = alloca i8*, align 8, !intel_dtrans_type ![[VOID_PTR:[0-9]+]]
// CHECK: %[[ARC0:.+]] = alloca %struct.arc**, align 8, !intel_dtrans_type ![[ARC_PTR_PTR]]
// CHECK: %[[ARC1:.+]] = alloca %struct.arc**, align 8, !intel_dtrans_type ![[ARC_PTR_PTR]]
// CHECK: %[[FUNC_LOAD:.+]] = load i32 (%struct.arc**, %struct.arc**)*, i32 (%struct.arc**, %struct.arc**)** @arc_cmp
// CHECK: %[[ARC0_LOAD:.+]] = load %struct.arc**, %struct.arc*** %[[ARC0]]
// CHECK: %[[ARC1_LOAD:.+]] = load %struct.arc**, %struct.arc*** %[[ARC1]]
// CHECK: call i32 %[[FUNC_LOAD]](%struct.arc** %[[ARC0_LOAD]], %struct.arc** %[[ARC1_LOAD]]), !intel_dtrans_type ![[FUNC:[0-9]+]]

// CHECK: !intel.dtrans.types = !{![[ARC:[0-9]+]]}

// CHECK: ![[ARC_PTR_ARRAY]] = !{!"A", i32 2, ![[ARC_PTR:[0-9]+]]}
// CHECK: ![[ARC_PTR]] = !{%struct.arc zeroinitializer, i32 1}
// CHECK: ![[FPTR]] = !{![[FUNC]], i32 1}
// CHECK: ![[FUNC]] = !{!"F", i1 false, i32 2, ![[INT:[0-9]+]], ![[ARC_PTR_PTR]], ![[ARC_PTR_PTR]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[ARC]] = !{!"S", %struct.arc zeroinitializer, i32 3, ![[INT]], ![[LONG:[0-9]+]], ![[ARC_PTR]]}
// CHECK: ![[LONG]] = !{i64 0, i32 0}

// CHECK: ![[COSTLESS_FUNC_MD]] = distinct !{![[ARC_PTR_PTR]], ![[ARC_PTR_PTR]]}
// CHECK: ![[TEST_FUNC_MD]] = distinct !{![[VOID_PTR]]}
// CHECK: ![[VOID_PTR]] = !{i8 0, i32 1}
