// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -opaque-pointers -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

struct pmop {
  struct a *b;
} *c;

// CHECK: @c = global ptr null, align 8, !intel_dtrans_type ![[PTR_PMOP_STRUCT:[0-9]+]]

// CHECK: !intel.dtrans.types = !{![[PMOP_STRUCT:[0-9]+]], ![[A_STRUCT:[0-9]+]]}

// CHECK: ![[PTR_PMOP_STRUCT]] = !{%struct._ZTS4pmop.pmop zeroinitializer, i32 1}
// CHECK: ![[PMOP_STRUCT]] = !{!"S", %struct._ZTS4pmop.pmop zeroinitializer, i32 1, ![[PTR_STRUCT_A:[0-9]+]]}
// CHECK: ![[PTR_STRUCT_A]] = !{%struct._ZTS1a.a zeroinitializer, i32 1}
// CHECK: ![[A_STRUCT]] = !{!"S", %struct._ZTS1a.a zeroinitializer, i32 -1}

