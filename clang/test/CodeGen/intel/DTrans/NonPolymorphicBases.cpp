// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ

struct default_ctor {
  default_ctor () {};
};

struct NormalBase {
  default_ctor field1;
  int *field2;
  default_ctor field3;
};

struct Inherits : public NormalBase {};

struct HasRefToInherits {
  const Inherits &I;
  HasRefToInherits();
  void func();
};

void foo() {
  HasRefToInherits{}.func();
}



// PTR: %struct._ZTS16HasRefToInherits.HasRefToInherits = type { %struct._ZTS8Inherits.Inherits* }
// OPQ: %struct._ZTS16HasRefToInherits.HasRefToInherits = type { ptr }

// CHECK: %struct._ZTS8Inherits.Inherits = type { %struct._ZTS10NormalBase.NormalBase.base, [7 x i8] }

// PTR: %struct._ZTS10NormalBase.NormalBase.base = type <{ %struct._ZTS12default_ctor.default_ctor, [7 x i8], i32*, %struct._ZTS12default_ctor.default_ctor }>
// OPQ: %struct._ZTS10NormalBase.NormalBase.base = type <{ %struct._ZTS12default_ctor.default_ctor, [7 x i8], ptr, %struct._ZTS12default_ctor.default_ctor }>

// CHECK: %struct._ZTS12default_ctor.default_ctor = type { i8 }

// PTR: %struct._ZTS10NormalBase.NormalBase = type <{ %struct._ZTS12default_ctor.default_ctor, [7 x i8], i32*, %struct._ZTS12default_ctor.default_ctor, [7 x i8] }>
// OPQ: %struct._ZTS10NormalBase.NormalBase = type <{ %struct._ZTS12default_ctor.default_ctor, [7 x i8], ptr, %struct._ZTS12default_ctor.default_ctor, [7 x i8] }>

// PTR: declare !intel.dtrans.func.type ![[CTOR_MD:[0-9]+]] void @_ZN16HasRefToInheritsC1Ev(%struct._ZTS16HasRefToInherits.HasRefToInherits* {{.+}}"intel_dtrans_func_index"="1")
// OPQ: declare !intel.dtrans.func.type ![[CTOR_MD:[0-9]+]] void @_ZN16HasRefToInheritsC1Ev(ptr {{.+}}"intel_dtrans_func_index"="1")

// PTR: declare !intel.dtrans.func.type ![[FUNC_MD:[0-9]+]]  void @_ZN16HasRefToInherits4funcEv(%struct._ZTS16HasRefToInherits.HasRefToInherits* {{.+}}"intel_dtrans_func_index"="1")
// OPQ: declare !intel.dtrans.func.type ![[FUNC_MD:[0-9]+]]  void @_ZN16HasRefToInherits4funcEv(ptr {{.+}}"intel_dtrans_func_index"="1")

// PTR: !intel.dtrans.types = !{![[REFTOINH:[0-9]+]], ![[INHERITS:[0-9]+]], ![[NORMBASE_BASE:[0-9]+]], ![[DEF_CTOR:[0-9]+]], ![[NORMBASE:[0-9]+]]}
// OPQ: !intel.dtrans.types = !{![[REFTOINH:[0-9]+]], ![[INHERITS:[0-9]+]], ![[NORMBASE:[0-9]+]], ![[NORMBASE_BASE:[0-9]+]], ![[DEF_CTOR:[0-9]+]]}

// CHECK: ![[REFTOINH]] = !{!"S", %struct._ZTS16HasRefToInherits.HasRefToInherits zeroinitializer, i32 1, ![[INHERITS_PTR:[0-9]+]]}
// CHECK: ![[INHERITS_PTR]] = !{%struct._ZTS8Inherits.Inherits zeroinitializer, i32 1}
// CHECK: ![[INHERITS]] = !{!"S", %struct._ZTS8Inherits.Inherits zeroinitializer, i32 2, ![[NORMBASE_BASE_REF:[0-9]+]], ![[PADDING_ARR:[0-9]+]]}
// CHECK: ![[NORMBASE_BASE_REF]] = !{%struct._ZTS10NormalBase.NormalBase.base zeroinitializer, i32 0}
// CHECK: ![[PADDING_ARR]] = !{!"A", i32 7, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK-DAG: ![[NORMBASE_BASE]] = !{!"S", %struct._ZTS10NormalBase.NormalBase.base zeroinitializer, i32 4, ![[DEF_CTOR_REF:[0-9]+]], ![[PADDING_ARR]], ![[INT_PTR:[0-9]+]], ![[DEF_CTOR_REF]]}
// CHECK-DAG: ![[DEF_CTOR_REF]] = !{%struct._ZTS12default_ctor.default_ctor zeroinitializer, i32 0}
// CHECK-DAG: ![[INT_PTR]] = !{i32 0, i32 1}
// CHECK-DAG: ![[DEF_CTOR]] = !{!"S", %struct._ZTS12default_ctor.default_ctor zeroinitializer, i32 1, !6}
// CHECK-DAG: ![[NORMBASE]] = !{!"S", %struct._ZTS10NormalBase.NormalBase zeroinitializer, i32 5, ![[DEF_CTOR_REF]], ![[PADDING_ARR]], ![[INT_PTR]], ![[DEF_CTOR_REF]], ![[PADDING_ARR]]}
// CHECK: ![[CTOR_MD]] = distinct !{![[FUNC_W_THIS:[0-9]+]]}
// CHECK: ![[FUNC_W_THIS]] = !{%struct._ZTS16HasRefToInherits.HasRefToInherits zeroinitializer, i32 1}
// CHECK: ![[FUNC_MD]] = distinct !{![[FUNC_W_THIS:[0-9]+]]}

