// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
struct a {
  int b;
  long c;
};
struct B {
  struct d {
    enum {} e;
    a f;
  };
};
B (*g) (B::d);
// CHECK: @g = global void (%"struct.B::d"*)* null, align 8, !intel_dtrans_type ![[FUNC_PTR:[0-9]+]]

// CHECK: !intel.dtrans.types = !{![[STRUCTB_D:[0-9]+]], ![[STRUCT_A:[0-9]+]]}

// CHECK: ![[FUNC_PTR]] = !{![[FUNC:[0-9]+]], i32 1}
// CHECK: ![[FUNC]] = !{!"F", i1 false, i32 1, ![[VOID:[0-9]+]], ![[B_D_PTR:[0-9]+]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
// CHECK: ![[B_D_PTR]] = !{%"struct.B::d" zeroinitializer, i32 1}
// Note: The enum field just stores its underlying type, so just an int.
// CHECK: ![[STRUCTB_D]] = !{!"S", %"struct.B::d" zeroinitializer, i32 2, ![[INT:[0-9]+]], ![[STRUCT_A_REF:[0-9]+]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[STRUCT_A_REF]] = !{%struct.a zeroinitializer, i32 0}
// CHECK: ![[STRUCT_A]] = !{!"S", %struct.a zeroinitializer, i32 2, ![[INT]], ![[LONG:[0-9]+]]}
// CHECK: ![[LONG]] = !{i64 0, i32 0}
