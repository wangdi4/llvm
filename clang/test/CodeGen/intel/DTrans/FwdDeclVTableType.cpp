// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
class a;
class b {};
class c {
public:
  c(b *);
};
class d : public b {
  virtual a e();
};
class f {
  c g;
  f();
};
f::f() : g(new d) {}


// vtable for 'd'
// CHECK: @_ZTV1d = available_externally unnamed_addr constant {{.*}} !intel_dtrans_type ![[D_VTABLE:[0-9]+]]
// f::f()
// CHECK: define dso_local void @_ZN1fC2Ev(%class._ZTS1f.f* nonnull align 1 dereferenceable(1) "intel_dtrans_func_index"="1" %{{.*}}){{.*}}!intel.dtrans.func.type ![[F_CTOR:[0-9]+]]
// Operator new.
// CHECK: declare !intel.dtrans.func.type ![[OP_NEW:[0-9]+]] nonnull "intel_dtrans_func_index"="1" i8* @_Znwm(i64)
// d::d()
// CHECK: define linkonce_odr void @_ZN1dC1Ev(%class._ZTS1d.d* nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %{{.*}}){{.*}} !intel.dtrans.func.type ![[D_CTOR:[0-9]+]]
// c::c(b*)
// CHECK: declare !intel.dtrans.func.type ![[C_CTOR:[0-9]+]] void @_ZN1cC1EP1b(%class._ZTS1c.c* nonnull align 1 dereferenceable(1) "intel_dtrans_func_index"="1", %class._ZTS1b.b* "intel_dtrans_func_index"="2")

// CHECK: !intel.dtrans.types = !{![[F:[0-9]+]], ![[C:[0-9]+]], ![[D:[0-9]+]], ![[B:[0-9]+]]}

// CHECK: ![[D_VTABLE]] = !{!"L", i32 1, ![[ARR_CHAR_PTRS:[0-9]+]]}
// CHECK: ![[ARR_CHAR_PTRS]] = !{!"A", i32 3, ![[CHAR_PTR:[0-9]+]]}
// CHECK: ![[CHAR_PTR]] = !{i8 0, i32 1}
//
// CHECK: ![[F]] = !{!"S", %class._ZTS1f.f zeroinitializer, i32 1, ![[C_REF:[0-9]+]]}
// CHECK: ![[C_REF]] = !{%class._ZTS1c.c zeroinitializer, i32 0}
//
// CHECK: ![[C]] = !{!"S", %class._ZTS1c.c zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[D]] = !{!"S", %class._ZTS1d.d zeroinitializer, i32 1, ![[FPTR:[0-9]+]]}
// CHECK: ![[FPTR]] = !{![[FUNC:[0-9]+]], i32 2}
// CHECK: ![[FUNC]] = !{!"F", i1 true, i32 0, ![[INT:[0-9]+]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
//
// CHECK: ![[B]] = !{!"S", %class._ZTS1b.b zeroinitializer, i32 1, ![[CHAR]]}
//
// CHECK: ![[F_CTOR]] = distinct !{![[F_PTR:[0-9]+]]}
// CHECK: ![[F_PTR]] = !{%class._ZTS1f.f zeroinitializer, i32 1}
//
// CHECK: ![[OP_NEW]] = distinct !{![[CHAR_PTR]]}
//
// CHECK: ![[D_CTOR]] = distinct !{![[D_PTR:[0-9]+]]}
// CHECK: ![[D_PTR]] = !{%class._ZTS1d.d zeroinitializer, i32 1}
//
// CHECK: ![[C_CTOR]] = distinct !{![[C_PTR:[0-9]+]], ![[B_PTR:[0-9]+]]}
// CHECK: ![[C_PTR]] = !{%class._ZTS1c.c zeroinitializer, i32 1}
// CHECK: ![[B_PTR]] = !{%class._ZTS1b.b zeroinitializer, i32 1}
