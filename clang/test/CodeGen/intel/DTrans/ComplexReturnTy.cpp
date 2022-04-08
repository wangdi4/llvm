// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ

struct Foo {
_Complex float g();
_Complex float *g2();
};

void use() {
  Foo f;
  f.g();
  f.g2();
}

// PTR: declare !intel.dtrans.func.type ![[G:[0-9]+]] noundef <2 x float> @_ZN3Foo1gEv(%struct._ZTS3Foo.Foo* {{[^,]*}}"intel_dtrans_func_index"="1")
// OPQ: declare !intel.dtrans.func.type ![[G:[0-9]+]] noundef <2 x float> @_ZN3Foo1gEv(ptr {{[^,]*}}"intel_dtrans_func_index"="1")
// PTR: declare !intel.dtrans.func.type ![[G2:[0-9]+]] noundef "intel_dtrans_func_index"="1" { float, float }* @_ZN3Foo2g2Ev(%struct._ZTS3Foo.Foo* {{[^,]*}}"intel_dtrans_func_index"="2")
// OPQ: declare !intel.dtrans.func.type ![[G2:[0-9]+]] noundef "intel_dtrans_func_index"="1" ptr @_ZN3Foo2g2Ev(ptr {{[^,]*}}"intel_dtrans_func_index"="2")
// CHECK: !intel.dtrans.types = !{![[FOO:[0-9]+]]}
// CHECK: ![[FOO]] = !{!"S", %struct._ZTS3Foo.Foo zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[G]] = distinct !{![[FOO_PTR:[0-9]+]]}
// CHECK: ![[FOO_PTR]] = !{%struct._ZTS3Foo.Foo zeroinitializer, i32 1}
// CHECK: ![[G2]] = distinct !{![[FLOAT_FLOAT_LITERAL_PTR:[0-9]+]], ![[FOO_PTR]]}
// CHECK: ![[FLOAT_FLOAT_LITERAL_PTR]] = !{![[FF_LITERAL:[0-9]+]], i32 1}
// CHECK: ![[FF_LITERAL]] = !{!"L", i32 2, ![[FLOAT:[0-9]+]], ![[FLOAT]]}
