// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -fexceptions -fcxx-exceptions -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

// Ensure that the metadata is emitted correctly for the non-virtual thunks for
// the destructor, previously @_ZThn8_N1cD1Ev and @_ZThn8_N1cD0Ev were missing
// it. Windows doesn't generate thunks, so no reason to run this test against
// a windows target.

// test list 1, problem is the 2 destructor thunks.
class a {
public:
  virtual ~a();
};
class b {
public:
  virtual ~b();
};
class c : b, a {
  ~c();
};
c::~c() {}
// CHECK: define {{.*}}void @_ZN1cD2Ev(%class._ZTS1c.c* nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="1" %this){{.+}} !intel.dtrans.func.type ![[C_DTOR_MD:[0-9]+]]
// CHECK: define {{.*}}void @_ZThn8_N1cD1Ev(%class._ZTS1c.c* "intel_dtrans_func_index"="1" %this){{.+}} !intel.dtrans.func.type ![[THUNK1_MD:[0-9]+]]
// CHECK: define {{.*}}void @_ZThn8_N1cD0Ev(%class._ZTS1c.c* "intel_dtrans_func_index"="1" %this){{.+}} !intel.dtrans.func.type ![[THUNK0_MD:[0-9]+]]

// test list 2, problem is thunk for NS:f::e
namespace NS {
  class b;
  class c {
  public:
      virtual ~c();
  };
  class d {
      virtual void e(const b &, bool, bool);
  };
  class f : c, d {
      void e(const b &, bool, bool);
  };
  void f::e(const b &, bool, bool) {}
} // namespace a

// CHECK: define {{.*}}void @_ZN2NS1f1eERKNS_1bEbb(%"class._ZTSN2NS1fE.NS::f"* nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="1" %this, %"class._ZTSN2NS1bE.NS::b"* nonnull align 1 "intel_dtrans_func_index"="2" %0, i1 zeroext %1, i1 zeroext %2){{.*}}!intel.dtrans.func.type ![[E_MD:[0-9]+]]
// CHECK: define {{.*}}void @_ZThn8_N2NS1f1eERKNS_1bEbb(%"class._ZTSN2NS1fE.NS::f"* "intel_dtrans_func_index"="1" %this, %"class._ZTSN2NS1bE.NS::b"* nonnull align 1 "intel_dtrans_func_index"="2" %0, i1 zeroext %1, i1 zeroext %2){{.+}} !intel.dtrans.func.type ![[E_THNK_MD:[0-9]+]]

// CHECK: ![[C_DTOR_MD]] = distinct !{![[C_PTR:[0-9]+]]}
// CHECK: ![[C_PTR]] = !{%{{\"?}}class.{{.*}}.c{{\"?}} zeroinitializer, i32 1}
// CHECK: ![[THUNK1_MD]] = distinct !{![[C_PTR]]}
// CHECK: ![[THUNK0_MD]] = distinct !{![[C_PTR]]}

// CHECK: ![[E_MD]] = distinct !{![[F_PTR:[0-9]+]], ![[B_PTR:[0-9]+]]}
// CHECK: ![[F_PTR]] = !{%"class._ZTSN2NS1fE.NS::f" zeroinitializer, i32 1}
// CHECK: ![[B_PTR]] = !{%"class._ZTSN2NS1bE.NS::b" zeroinitializer, i32 1}
// CHECK: ![[E_THNK_MD]] = distinct !{![[F_PTR]], ![[B_PTR]]}
