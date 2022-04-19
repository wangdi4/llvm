// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ

class Base {
public:
  virtual ~Base();
};
class B{};

class Derived : public Base {
};

int main() {
  Derived instance;
}

// PTR: @_ZTV7Derived = linkonce_odr unnamed_addr constant { [4 x i8*] }{{.+}} !intel_dtrans_type ![[VTABLE:[0-9]+]]
// OPQ: @_ZTV7Derived = linkonce_odr unnamed_addr constant { [4 x ptr] }{{.+}} !intel_dtrans_type ![[VTABLE:[0-9]+]]
// PTR: @_ZTI7Derived = linkonce_odr constant { i8*, i8*, i8* }{{.+}} !intel_dtrans_type ![[TYPEINFO:[0-9]+]]
// OPQ: @_ZTI7Derived = linkonce_odr constant { ptr, ptr, ptr }{{.+}} !intel_dtrans_type ![[TYPEINFO:[0-9]+]]
// PTR: @_ZTV4Base = available_externally unnamed_addr constant { [4 x i8*] }{{.+}} !intel_dtrans_type ![[VTABLE]]
// OPQ: @_ZTV4Base = available_externally unnamed_addr constant { [4 x ptr] }{{.+}} !intel_dtrans_type ![[VTABLE]]

// PTR: define linkonce_odr void @_ZN7DerivedC1Ev(%class._ZTS7Derived.Derived* {{[^,]*}}"intel_dtrans_func_index"="1" %this) {{.*}}!intel.dtrans.func.type ![[DERIVED_CTOR_FUNC_MD:[0-9]+]]
// OPQ: define linkonce_odr void @_ZN7DerivedC1Ev(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %this) {{.*}}!intel.dtrans.func.type ![[DERIVED_CTOR_FUNC_MD:[0-9]+]]
// PTR: alloca %class._ZTS7Derived.Derived*, align 8, !intel_dtrans_type ![[DERIVED_PTR:[0-9]+]]
// OPQ: alloca ptr, align 8, !intel_dtrans_type ![[DERIVED_PTR:[0-9]+]]
// PTR: define linkonce_odr void @_ZN7DerivedD1Ev(%class._ZTS7Derived.Derived* {{[^,]*}}"intel_dtrans_func_index"="1" %this) {{.*}}!intel.dtrans.func.type ![[DERIVED_DTOR1_FUNC_MD:[0-9]+]]
// OPQ: define linkonce_odr void @_ZN7DerivedD1Ev(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %this) {{.*}}!intel.dtrans.func.type ![[DERIVED_DTOR1_FUNC_MD:[0-9]+]]
// PTR: alloca %class._ZTS7Derived.Derived*, align 8, !intel_dtrans_type ![[DERIVED_PTR]]
// OPQ: alloca ptr, align 8, !intel_dtrans_type ![[DERIVED_PTR]]
// PTR: define linkonce_odr void @_ZN7DerivedC2Ev(%class._ZTS7Derived.Derived* {{[^,]*}}"intel_dtrans_func_index"="1" %this) {{.*}}!intel.dtrans.func.type ![[DERIVED_CTOR2_FUNC_MD:[0-9]+]]
// OPQ: define linkonce_odr void @_ZN7DerivedC2Ev(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %this) {{.*}}!intel.dtrans.func.type ![[DERIVED_CTOR2_FUNC_MD:[0-9]+]]
// PTR: alloca %class._ZTS7Derived.Derived*, align 8, !intel_dtrans_type ![[DERIVED_PTR]]
// OPQ: alloca ptr, align 8, !intel_dtrans_type ![[DERIVED_PTR]]
// PTR: define linkonce_odr void @_ZN4BaseC2Ev(%class._ZTS4Base.Base* {{[^,]*}}"intel_dtrans_func_index"="1" %this) {{.*}}!intel.dtrans.func.type ![[BASE_CTOR2_FUNC_MD:[0-9]+]]
// OPQ: define linkonce_odr void @_ZN4BaseC2Ev(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %this) {{.*}}!intel.dtrans.func.type ![[BASE_CTOR2_FUNC_MD:[0-9]+]]
// PTR: alloca %class._ZTS4Base.Base*, align 8, !intel_dtrans_type ![[BASE_PTR:[0-9]+]]
// OPQ: alloca ptr, align 8, !intel_dtrans_type ![[BASE_PTR:[0-9]+]]
// PTR: define linkonce_odr void @_ZN7DerivedD0Ev(%class._ZTS7Derived.Derived* {{[^,]*}}"intel_dtrans_func_index"="1" %this) {{.*}}!intel.dtrans.func.type ![[DERIVED_DTOR0_FUNC_MD:[0-9]+]]
// OPQ: define linkonce_odr void @_ZN7DerivedD0Ev(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %this) {{.*}}!intel.dtrans.func.type ![[DERIVED_DTOR0_FUNC_MD:[0-9]+]]
// PTR: alloca %class._ZTS7Derived.Derived*, align 8, !intel_dtrans_type ![[DERIVED_PTR]]
// OPQ: alloca ptr, align 8, !intel_dtrans_type ![[DERIVED_PTR]]
// PTR: declare !intel.dtrans.func.type ![[OP_DELETE_FUNC_MD:[0-9]+]] void @_ZdlPv(i8* noundef "intel_dtrans_func_index"="1")
// OPQ: declare !intel.dtrans.func.type ![[OP_DELETE_FUNC_MD:[0-9]+]] void @_ZdlPv(ptr noundef "intel_dtrans_func_index"="1")
// PTR: define linkonce_odr void @_ZN7DerivedD2Ev(%class._ZTS7Derived.Derived* {{[^,]*}}"intel_dtrans_func_index"="1" %this) {{.*}}!intel.dtrans.func.type ![[DERIVED_DTOR2_FUNC_MD:[0-9]+]]
// OPQ: define linkonce_odr void @_ZN7DerivedD2Ev(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %this) {{.*}}!intel.dtrans.func.type ![[DERIVED_DTOR2_FUNC_MD:[0-9]+]]
// PTR: alloca %class._ZTS7Derived.Derived*, align 8, !intel_dtrans_type ![[DERIVED_PTR]]
// OPQ: alloca ptr, align 8, !intel_dtrans_type ![[DERIVED_PTR]]
// PTR: declare !intel.dtrans.func.type ![[BASE_DTOR2_FUNC_MD:[0-9]+]] void @_ZN4BaseD2Ev(%class._ZTS4Base.Base* {{[^,]*}}"intel_dtrans_func_index"="1")
// OPQ: declare !intel.dtrans.func.type ![[BASE_DTOR2_FUNC_MD:[0-9]+]] void @_ZN4BaseD2Ev(ptr {{[^,]*}}"intel_dtrans_func_index"="1")

// CHECK: !intel.dtrans.types = !{![[DERIVED:[0-9]+]], ![[BASE:[0-9]+]]}

// CHECK: ![[VTABLE]] = !{!"L", i32 1, ![[VTABLE_ARRAY:[0-9]+]]}
// CHECK: ![[VTABLE_ARRAY]] = !{!"A", i32 4, ![[CHAR_PTR:[0-9]+]]}
// CHECK: ![[CHAR_PTR]] = !{i8 0, i32 1}
// CHECK: ![[TYPEINFO]] = !{!"L", i32 3, ![[CHAR_PTR]], ![[CHAR_PTR]], ![[CHAR_PTR]]}


// CHECK: ![[DERIVED]] = !{!"S", %class._ZTS7Derived.Derived zeroinitializer, i32 1, ![[BASEELEM:[0-9]+]]}
// CHECK: ![[BASEELEM]] = !{%class._ZTS4Base.Base zeroinitializer, i32 0}

// CHECK: ![[BASE]] = !{!"S", %class._ZTS4Base.Base zeroinitializer, i32 1, ![[VFPTR:[0-9]+]]}
// CHECK: ![[VFPTR]] = !{![[FUNC:[0-9]+]], i32 2}
// CHECK: ![[FUNC]] = !{!"F", i1 true, i32 0, ![[INT:[0-9]+]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}


// CHECK: ![[DERIVED_CTOR_FUNC_MD]]  = distinct !{![[DERIVED_PTR]]}
// CHECK: ![[DERIVED_PTR]] = !{%class._ZTS7Derived.Derived zeroinitializer, i32 1}
// CHECK: ![[DERIVED_DTOR1_FUNC_MD]] = distinct !{![[DERIVED_PTR]]}
// CHECK: ![[DERIVED_CTOR2_FUNC_MD]] = distinct !{![[DERIVED_PTR]]}
// CHECK: ![[BASE_CTOR2_FUNC_MD]] = distinct !{![[BASE_PTR]]}
// CHECK: ![[BASE_PTR]] = !{%class._ZTS4Base.Base zeroinitializer, i32 1}
// CHECK: ![[DERIVED_DTOR0_FUNC_MD]] = distinct !{![[DERIVED_PTR]]}
// CHECK: ![[OP_DELETE_FUNC_MD]] = distinct !{![[CHAR_PTR]]}
// CHECK: ![[DERIVED_DTOR2_FUNC_MD]] = distinct !{![[DERIVED_PTR]]}
// CHECK: ![[BASE_DTOR2_FUNC_MD]] = distinct !{![[BASE_PTR]]}

