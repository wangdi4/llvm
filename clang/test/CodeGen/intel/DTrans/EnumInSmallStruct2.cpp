// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
struct a {
  enum { T1,
         T2 } c;
  double b;
  char d;
};
a (*e)(int);
// CHECK: @e = global void (%struct.a*, i32)* null, align 8, !intel_dtrans_type ![[FUNC_PTR:[0-9]+]]

// CHECK: !intel.dtrans.types = !{![[STRUCTA:[0-9]+]]}
// CHECK: ![[FUNC_PTR]] = !{![[FUNC:[0-9]+]], i32 1}
// CHECK: ![[FUNC]] = !{!"F", i1 false, i32 2, ![[VOID:[0-9]+]], ![[STRUCTA_PTR:[0-9]+]], ![[INT:[0-9]+]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
// CHECK: ![[STRUCTA_PTR]] = !{%struct.a zeroinitializer, i32 1}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[STRUCTA]] = !{!"S", %struct.a zeroinitializer, i32 3, ![[INT]], ![[DOUBLE:[0-9]+]], ![[CHAR:[0-9]+]]}
// CHECK: ![[DOUBLE]] = !{double 0.0{{.+}}, i32 0}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
