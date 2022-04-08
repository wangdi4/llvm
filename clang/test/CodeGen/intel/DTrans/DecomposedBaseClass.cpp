// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
template <typename a> struct b {
  int c;
  a d;
};
class e {
public:
  class f : b<int *> {};
  e(int, int);
  f g();
};
int h;
void i() {
  e j(h, 0);
  j.g();
}

// e::e(int,int)
// PTR: declare !intel.dtrans.func.type ![[E_CTOR:[0-9]+]] void @_ZN1eC1Eii(%class._ZTS1e.e* {{[^,]*}}"intel_dtrans_func_index"="1", i32 noundef, i32 noundef)
// OPQ: declare !intel.dtrans.func.type ![[E_CTOR:[0-9]+]] void @_ZN1eC1Eii(ptr {{[^,]*}}"intel_dtrans_func_index"="1", i32 noundef, i32 noundef)
// PTR: declare !intel.dtrans.func.type ![[G:[0-9]+]] "intel_dtrans_func_index"="1" { i32, i32* } @_ZN1e1gEv(%class._ZTS1e.e* {{[^,]*}}"intel_dtrans_func_index"="2")
// OPQ: declare !intel.dtrans.func.type ![[G:[0-9]+]] "intel_dtrans_func_index"="1" { i32, ptr } @_ZN1e1gEv(ptr {{[^,]*}}"intel_dtrans_func_index"="2")

// CHECK: intel.dtrans.types = !{![[E:[0-9]+]], ![[F:[0-9]+]], ![[B:[0-9]+]]}
// CHECK: ![[E]] = !{!"S", %class._ZTS1e.e zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[F]] = !{!"S", %"class._ZTSN1e1fE.e::f" zeroinitializer, i32 1, ![[B_REF:[0-9]+]]}
// CHECK: ![[B_REF]] = !{%struct._ZTS1bIPiE.b zeroinitializer, i32 0}
// CHECK: ![[B]] = !{!"S", %struct._ZTS1bIPiE.b zeroinitializer, i32 2, ![[INT:[0-9]+]], ![[INT_PTR:[0-9]+]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[INT_PTR]] = !{i32 0, i32 1}
// CHECK: ![[E_CTOR]] = distinct !{![[E_PTR:[0-9]+]]}
// CHECK: ![[E_PTR]] = !{%class._ZTS1e.e zeroinitializer, i32 1}
// CHECK: ![[G]] = distinct !{![[G_LITERAL:[0-9]+]], ![[E_PTR]]}
// CHECK: ![[G_LITERAL]] = !{!"L", i32 2, ![[INT]], ![[INT_PTR]]}
