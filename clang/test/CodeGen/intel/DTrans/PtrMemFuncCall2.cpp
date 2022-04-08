// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
class a {};
class b {
  typedef void (a::*c)(short, int);
  void d();
  static void e() {
    a g;
    c f;
    (g.*f)(0, int());
  }
};
void b::d() { (void)e; }

// PTR: define dso_local void @_ZN1b1dEv(%class._ZTS1b.b* {{[^,]*}}"intel_dtrans_func_index"="1" %this){{.+}}!intel.dtrans.func.type ![[B_D:[0-9]+]]
// OPQ: define dso_local void @_ZN1b1dEv(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %this){{.+}}!intel.dtrans.func.type ![[B_D:[0-9]+]]
// PTR: alloca %class._ZTS1b.b*, align 8, !intel_dtrans_type ![[B_PTR:[0-9]+]]
// OPQ: alloca ptr, align 8, !intel_dtrans_type ![[B_PTR:[0-9]+]]
// PTR:  call void %{{.+}}(%class._ZTS1a.a* {{[^,]*}} %this.adjusted, i16 noundef signext 0, i32 noundef 0), !intel_dtrans_type ![[PMF:[0-9]+]]
// OPQ:  call void %{{.+}}(ptr {{[^,]*}}, i16 noundef signext 0, i32 noundef 0), !intel_dtrans_type ![[PMF:[0-9]+]]

// CHECK: !intel.dtrans.types = !{![[B:[0-9]+]], ![[A:[0-9]+]]}

// CHECK: ![[B]] = !{!"S", %class._ZTS1b.b zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[A]] = !{!"S", %class._ZTS1a.a zeroinitializer, i32 1, ![[CHAR]]}
// CHECK: ![[B_D]] = distinct !{![[B_PTR]]}
// CHECK: ![[B_PTR]] = !{%class._ZTS1b.b zeroinitializer, i32 1}

// CHECK: ![[PMF]] = !{!"F", i1 false, i32 3, ![[VOID:[0-9]+]], ![[A_PTR:[0-9]+]], ![[SHORT:[0-9]+]], ![[INT:[0-9]+]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
// CHECK: ![[A_PTR]] = !{%class._ZTS1a.a zeroinitializer, i32 1}
// CHECK: ![[SHORT]] = !{i16 0, i32 0}
// CHECK: ![[INT]] = !{i32 0, i32 0}
