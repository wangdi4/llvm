// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
struct a {
  float b;
  float c;
};
a (*d)();

// CHECK: @d = global <2 x float> ()* null, align 8, !intel_dtrans_type ![[D:[0-9]+]]

// CHECK: !intel.dtrans.types = !{}
// CHECK: ![[D]] = !{![[FUNCTY:[0-9]+]], i32 1}
// CHECK: ![[FUNCTY]] = !{!"F", i1 false, i32 0, ![[VECTY:[0-9]+]]}
// CHECK: ![[VECTY]] = !{!"V", i32 2, ![[FLOAT:[0-9]+]]}
// CHECK: ![[FLOAT]] = !{float 0.0{{.+}}, i32 0}
