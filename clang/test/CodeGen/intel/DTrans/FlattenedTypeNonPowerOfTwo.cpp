// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -mconstructor-aliases -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK
struct ObjectSizeOpts {
  char EvalMode = 0;
  bool RoundToAlign = false;
  bool NullIsUnknownSize = false;
};

struct ObjectSizeOffsetVisitor {
  ObjectSizeOffsetVisitor(ObjectSizeOpts Options = {});
};

ObjectSizeOffsetVisitor::ObjectSizeOffsetVisitor( ObjectSizeOpts Options)
      { }

// CHECK: define dso_local void @_ZN23ObjectSizeOffsetVisitorC2E14ObjectSizeOpts(ptr {{.*}}"intel_dtrans_func_index"="1" %{{.*}}, i24 %{{.*}}) {{.*}}!intel.dtrans.func.type ![[CTOR:[0-9]+]]

void foo() {
  ObjectSizeOffsetVisitor Visitor{};
  // CHECK: call void @_ZN23ObjectSizeOffsetVisitorC1E14ObjectSizeOpts(ptr {{.*}}%{{.*}}, i24 %{{.*}}){{.*}} !intel_dtrans_type ![[FPTR_TYPE:[0-9]+]]
}

// CHECK: !intel.dtrans.types = !{![[OPTS:[0-9]+]], ![[OFFSET_VISITOR:[0-9]+]]}
// CHECK: ![[OPTS]] = !{!"S", %struct._ZTS14ObjectSizeOpts.ObjectSizeOpts zeroinitializer, i32 3, ![[CHAR:[0-9]+]], ![[CHAR]], ![[CHAR]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[OFFSET_VISITOR]] = !{!"S", %struct._ZTS23ObjectSizeOffsetVisitor.ObjectSizeOffsetVisitor zeroinitializer, i32 1, ![[CHAR]]}
// CHECK: ![[CTOR]] = distinct !{![[VISITOR_PTR:[0-9]+]]}
// CHECK: ![[VISITOR_PTR]] = !{%struct._ZTS23ObjectSizeOffsetVisitor.ObjectSizeOffsetVisitor zeroinitializer, i32 1}
// CHECK: ![[FPTR_TYPE]] = !{!"F", i1 false, i32 2, ![[VOID:[0-9]+]], ![[VISITOR_PTR]], ![[I24:[0-9]+]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
// CHECK: ![[I24]] = !{i24 0, i32 0}
