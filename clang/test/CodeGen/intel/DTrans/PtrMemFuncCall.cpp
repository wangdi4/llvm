// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

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

// CHECK: define linkonce_odr void @_ZN6HelperC1Ev(%struct.Helper* nonnull align 8 dereferenceable(17) "intel_dtrans_func_index"="1" %this){{.+}}!intel.dtrans.func.type ![[CTOR1:[0-9]+]]
// CHECK: alloca %struct.Helper*, align 8, !intel_dtrans_type ![[HELPER_PTR:[0-9]+]]
// CHECK: define linkonce_odr void @_ZN6HelperC2Ev(%struct.Helper* nonnull align 8 dereferenceable(17) "intel_dtrans_func_index"="1" %this){{.+}}!intel.dtrans.func.type ![[CTOR2:[0-9]+]]
// CHECK: alloca %struct.Helper*, align 8, !intel_dtrans_type ![[HELPER_PTR:[0-9]+]]
// CHECK: call void %{{.+}}(%struct.SomeClass* nonnull align 1 dereferenceable(1) %this.adjusted, i32 0), !intel_dtrans_type ![[PMFCALL:[0-9]+]]

// CHECK: !intel.dtrans.types = !{![[HELPER:[0-9]+]], ![[SOMECLASS:[0-9]+]]}

// CHECK: ![[HELPER]] = !{!"S", %struct.Helper zeroinitializer, i32 3, ![[PMF_LITERAL_REF:[0-9]+]], ![[SOMECLASS_REF:[0-9]+]], ![[PADDING:[0-9]+]]}
// CHECK: ![[PMF_LITERAL_REF]] = !{![[PMF_LITERAL:[0-9]+]], i32 0}
// CHECK: ![[PMF_LITERAL]] = !{!"L", i32 2, ![[I64:[0-9]+]], ![[I64]]}
// CHECK: ![[I64]] = !{i64 0, i32 0}
// CHECK: ![[SOMECLASS_REF]] = !{%struct.SomeClass zeroinitializer, i32 0}
// CHECK: ![[PADDING]] = !{!"A", i32 7, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[SOMECLASS]] = !{!"S", %struct.SomeClass zeroinitializer, i32 1, ![[CHAR]]}
// CHECK: ![[CTOR1]] = distinct !{![[HELPER_PTR]]}
// CHECK: ![[HELPER_PTR]] = !{%struct.Helper zeroinitializer, i32 1}
// CHECK: ![[CTOR2]] = distinct !{![[HELPER_PTR]]}
// CHECK: ![[PMFCALL]] = !{!"F", i1 false, i32 2, ![[VOID:[0-9]+]], ![[SOMECLASS_PTR:[0-9]+]], ![[INT:[0-9]+]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
// CHECK: ![[SOMECLASS_PTR]] = !{%struct.SomeClass zeroinitializer, i32 1}
// CHECK: ![[INT]] = !{i32 0, i32 0}
