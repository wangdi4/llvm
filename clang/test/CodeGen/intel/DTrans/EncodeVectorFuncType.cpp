// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
struct a {
  float b;
  float c;
};
a (*d)();

struct a2 {
  float b[2];
};
a2 (*d2)();

struct a3 {
  __complex__ float b[1];
};
a3 (*d3)();

struct a4 {
  a3 bb;
};
a4 (*d4)();

// OPQ: @d = global ptr null, align 8, !intel_dtrans_type ![[D:[0-9]+]]
// OPQ: @d2 = global ptr null, align 8, !intel_dtrans_type ![[D]]
// OPQ: @d3 = global ptr null, align 8, !intel_dtrans_type ![[D]]
// OPQ: @d4 = global ptr null, align 8, !intel_dtrans_type ![[D]]

// CHECK: !intel.dtrans.types = !{}
// CHECK: ![[D]] = !{![[FUNCTY:[0-9]+]], i32 1}
// CHECK: ![[FUNCTY]] = !{!"F", i1 false, i32 0, ![[VECTY:[0-9]+]]}
// CHECK: ![[VECTY]] = !{!"V", i32 2, ![[FLOAT:[0-9]+]]}
// CHECK: ![[FLOAT]] = !{float 0.0{{.+}}, i32 0}
