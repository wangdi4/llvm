// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
struct a {
  enum { T1,
         T2 } c;
  double b;
  char d;
};
a (*e)(int);
// PTR: @e = global void (%struct._ZTS1a.a*, i32)* null, align 8, !intel_dtrans_type ![[FUNC_PTR:[0-9]+]]
// OPQ: @e = global ptr null, align 8, !intel_dtrans_type ![[FUNC_PTR:[0-9]+]]

// CHECK: !intel.dtrans.types = !{![[STRUCTA:[0-9]+]]}
// CHECK: ![[FUNC_PTR]] = !{![[FUNC:[0-9]+]], i32 1}
// CHECK: ![[FUNC]] = !{!"F", i1 false, i32 2, ![[VOID:[0-9]+]], ![[STRUCTA_PTR:[0-9]+]], ![[INT:[0-9]+]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
// CHECK: ![[STRUCTA_PTR]] = !{%struct._ZTS1a.a zeroinitializer, i32 1}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[STRUCTA]] = !{!"S", %struct._ZTS1a.a zeroinitializer, i32 3, ![[INT]], ![[DOUBLE:[0-9]+]], ![[CHAR:[0-9]+]]}
// CHECK: ![[DOUBLE]] = !{double 0.0{{.+}}, i32 0}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
