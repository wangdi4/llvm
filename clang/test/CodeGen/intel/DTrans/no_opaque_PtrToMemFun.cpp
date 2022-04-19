// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
class a;
template <class b> class c {
public:
  c();
  c(a *, void (b::*)());
};
class a {
  void d();
  void e();
};
void a::d() { c<a>(this, &a::e); }

// a::d()
// PTR: define dso_local void @_ZN1a1dEv(%class._ZTS1a.a* {{[^,]*}}"intel_dtrans_func_index"="1" %{{.*}}){{.*}}!intel.dtrans.func.type ![[A_D:[0-9]+]]
// OPQ: define dso_local void @_ZN1a1dEv(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %{{.*}}){{.*}}!intel.dtrans.func.type ![[A_D:[0-9]+]]
// a::e()
// PTR: declare !intel.dtrans.func.type ![[A_E:[0-9]+]] void @_ZN1a1eEv(%class._ZTS1a.a* {{[^,]*}}"intel_dtrans_func_index"="1")
// OPQ: declare !intel.dtrans.func.type ![[A_E:[0-9]+]] void @_ZN1a1eEv(ptr {{[^,]*}}"intel_dtrans_func_index"="1")
// c<a>::c(a*, void (a::*)())
// PTR: declare !intel.dtrans.func.type ![[C_C:[0-9]+]] void @_ZN1cI1aEC1EPS0_MS0_FvvE(%class._ZTS1cI1aE.c* {{[^,]*}}"intel_dtrans_func_index"="1", %class._ZTS1a.a* noundef "intel_dtrans_func_index"="2", i64, i64)
// OPQ: declare !intel.dtrans.func.type ![[C_C:[0-9]+]] void @_ZN1cI1aEC1EPS0_MS0_FvvE(ptr {{[^,]*}}"intel_dtrans_func_index"="1", ptr  noundef "intel_dtrans_func_index"="2", i64, i64)

// CHECK: intel.dtrans.types = !{![[A:[0-9]+]], ![[C:[0-9]+]]}
// CHECK: ![[A]] = !{!"S", %class._ZTS1a.a zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[C]] = !{!"S", %class._ZTS1cI1aE.c zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK: ![[A_D]] = distinct !{![[A_PTR:[0-9]+]]}
// CHECK: ![[A_PTR]] = !{%class._ZTS1a.a zeroinitializer, i32 1}
// CHECK: ![[A_E]] = distinct !{![[A_PTR]]}
// CHECK: ![[C_C]] = distinct !{![[C_PTR:[0-9]+]], ![[A_PTR]]}
// CHECK: ![[C_PTR]] = !{%class._ZTS1cI1aE.c zeroinitializer, i32 1}
