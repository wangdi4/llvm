// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s

class g {
  using MemPtr = void (g::*)();
  MemPtr i;
};

g my_g;

// CHECK: !intel.dtrans.types = !{![[G:[0-9]+]]}

// CHECK: ![[G]] = !{!"S", %class._ZTS1g.g zeroinitializer, i32 1, ![[LITERAL_REF:[0-9]+]]}
// CHECK: ![[LITERAL_REF]] = !{![[LITERAL:[0-9]+]], i32 0}
// CHECK: ![[LITERAL]] = !{!"L", i32 2, ![[I64:[0-9]+]], ![[I64]]}
