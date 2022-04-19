// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s

typedef union {
  unsigned long UL;
  void *PTR;
} A;

typedef union {
  void *PTR;
  unsigned long UL;
} B;

typedef union {
  unsigned short UL;
  void *PTR;
} C;

typedef union {
  void *PTR;
  unsigned short UL;
} D;

A a;
B b;
C c;
D d;

// CHECK: !intel.dtrans.types = !{![[A:[0-9]+]], ![[B:[0-9]+]], ![[C:[0-9]+]], ![[D:[0-9]+]]}
// CHECK: ![[A]] = !{!"S", %union._ZTS1A.A zeroinitializer, i32 1, ![[LONG:[0-9]+]]}
// CHECK: ![[LONG]] = !{i64 0, i32 0}
// CHECK: ![[B]] = !{!"S", %union._ZTS1B.B zeroinitializer, i32 1, ![[VOID_PTR:[0-9]+]]}
// CHECK: ![[VOID_PTR]] = !{i8 0, i32 1}
// CHECK: ![[C]] = !{!"S", %union._ZTS1C.C zeroinitializer, i32 1, ![[VOID_PTR:[0-9]+]]}
// CHECK: ![[D]] = !{!"S", %union._ZTS1D.D zeroinitializer, i32 1, ![[VOID_PTR:[0-9]+]]}
