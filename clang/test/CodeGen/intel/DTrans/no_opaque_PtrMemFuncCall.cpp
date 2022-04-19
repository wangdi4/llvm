// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ

struct SomeClass {
};

struct Helper {
  using MemPtrTy = void (SomeClass::*)(int);
  MemPtrTy memfuncptr;
  SomeClass classptr;
  Helper() {
    (&classptr->*memfuncptr)(0);
  }
};

void use() {
  Helper();
}

// PTR: define linkonce_odr void @_ZN6HelperC1Ev(%struct._ZTS6Helper.Helper* {{[^,]*}}"intel_dtrans_func_index"="1" %this){{.+}}!intel.dtrans.func.type ![[CTOR1:[0-9]+]]
// OPQ: define linkonce_odr void @_ZN6HelperC1Ev(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %this){{.+}}!intel.dtrans.func.type ![[CTOR1:[0-9]+]]
// PTR: alloca %struct._ZTS6Helper.Helper*, align 8, !intel_dtrans_type ![[HELPER_PTR:[0-9]+]]
// OPQ: alloca ptr, align 8, !intel_dtrans_type ![[HELPER_PTR:[0-9]+]]
// PTR: define linkonce_odr void @_ZN6HelperC2Ev(%struct._ZTS6Helper.Helper* {{[^,]*}}"intel_dtrans_func_index"="1" %this){{.+}}!intel.dtrans.func.type ![[CTOR2:[0-9]+]]
// OPQ: define linkonce_odr void @_ZN6HelperC2Ev(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %this){{.+}}!intel.dtrans.func.type ![[CTOR2:[0-9]+]]
// PTR: alloca %struct._ZTS6Helper.Helper*, align 8, !intel_dtrans_type ![[HELPER_PTR:[0-9]+]]
// OPQ: alloca ptr, align 8, !intel_dtrans_type ![[HELPER_PTR:[0-9]+]]
// PTR: call void %{{.+}}(%struct._ZTS9SomeClass.SomeClass* {{[^,]*}} %this.adjusted, i32 noundef 0), !intel_dtrans_type ![[PMFCALL:[0-9]+]]
// OPQ: call void %{{.+}}(ptr {{[^,]*}}, i32 noundef 0), !intel_dtrans_type ![[PMFCALL:[0-9]+]]
// CHECK: !intel.dtrans.types = !{![[HELPER:[0-9]+]], ![[SOMECLASS:[0-9]+]]}

// CHECK: ![[HELPER]] = !{!"S", %struct._ZTS6Helper.Helper zeroinitializer, i32 3, ![[PMF_LITERAL_REF:[0-9]+]], ![[SOMECLASS_REF:[0-9]+]], ![[PADDING:[0-9]+]]}
// CHECK: ![[PMF_LITERAL_REF]] = !{![[PMF_LITERAL:[0-9]+]], i32 0}
// CHECK: ![[PMF_LITERAL]] = !{!"L", i32 2, ![[I64:[0-9]+]], ![[I64]]}
// CHECK: ![[I64]] = !{i64 0, i32 0}
// CHECK: ![[SOMECLASS_REF]] = !{%struct._ZTS9SomeClass.SomeClass zeroinitializer, i32 0}
// CHECK: ![[PADDING]] = !{!"A", i32 7, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[SOMECLASS]] = !{!"S", %struct._ZTS9SomeClass.SomeClass zeroinitializer, i32 1, ![[CHAR]]}
// CHECK: ![[CTOR1]] = distinct !{![[HELPER_PTR]]}
// CHECK: ![[HELPER_PTR]] = !{%struct._ZTS6Helper.Helper zeroinitializer, i32 1}
// CHECK: ![[CTOR2]] = distinct !{![[HELPER_PTR]]}
// CHECK: ![[PMFCALL]] = !{!"F", i1 false, i32 2, ![[VOID:[0-9]+]], ![[SOMECLASS_PTR:[0-9]+]], ![[INT:[0-9]+]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
// CHECK: ![[SOMECLASS_PTR]] = !{%struct._ZTS9SomeClass.SomeClass zeroinitializer, i32 1}
// CHECK: ![[INT]] = !{i32 0, i32 0}
