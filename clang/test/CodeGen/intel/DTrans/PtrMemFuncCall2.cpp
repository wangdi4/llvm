// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
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

// CHECK: define dso_local void @_ZN1b1dEv(%class.b* nonnull align 1 dereferenceable(1) "intel_dtrans_func_index"="1" %this){{.+}}!intel.dtrans.func.type ![[B_D:[0-9]+]]
// CHECK: alloca %class.b*, align 8, !intel_dtrans_type ![[B_PTR:[0-9]+]]
// CHECK:  call void %{{.+}}(%class.a* nonnull align 1 dereferenceable(1) %this.adjusted, i16 signext 0, i32 0), !intel_dtrans_type ![[PMF:[0-9]+]]

// CHECK: !intel.dtrans.types = !{![[B:[0-9]+]], ![[A:[0-9]+]]}

// CHECK: ![[B]] = !{!"S", %class.b zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[A]] = !{!"S", %class.a zeroinitializer, i32 1, ![[CHAR]]}
// CHECK: ![[B_D]] = distinct !{![[B_PTR]]}
// CHECK: ![[B_PTR]] = !{%class.b zeroinitializer, i32 1}

// CHECK: ![[PMF]] = !{!"F", i1 false, i32 3, ![[VOID:[0-9]+]], ![[A_PTR:[0-9]+]], ![[SHORT:[0-9]+]], ![[INT:[0-9]+]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
// CHECK: ![[A_PTR]] = !{%class.a zeroinitializer, i32 1}
// CHECK: ![[SHORT]] = !{i16 0, i32 0}
// CHECK: ![[INT]] = !{i32 0, i32 0}
