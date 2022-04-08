// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s
typedef struct {
  int a;
  int *b;
  void *c;
} (*d)();
typedef struct {
  d e;
} f;
f g;

// CHECK: !intel.dtrans.types = !{![[STRUCT_F:[0-9]+]], ![[STRUCT_ANON:[0-9]+]]}
// CHECK: ![[STRUCT_F]] = !{!"S", %struct._ZTS1f.f zeroinitializer, i32 1, ![[FUNC_PTR:[0-9]+]]}
// CHECK: ![[FUNC_PTR]] = !{![[FUNC:[0-9]+]], i32 1}
// CHECK: ![[FUNC]] = !{!"F", i1 true, i32 1, ![[VOID:[0-9]+]], ![[ANON_PTR:[0-9]+]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
// CHECK: ![[ANON_PTR]] = !{%"struct._ZTS3$_0.anon" zeroinitializer, i32 1}
// CHECK: ![[STRUCT_ANON]] = !{!"S", %"struct._ZTS3$_0.anon" zeroinitializer, i32 3, ![[INT:[0-9]+]], ![[INT_PTR:[0-9]+]], ![[VOID_PTR:[0-9]+]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[INT_PTR]] = !{i32 0, i32 1}
// CHECK: ![[VOID_PTR]] = !{i8 0, i32 1}
