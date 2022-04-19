// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -opaque-pointers -emit-llvm %s -o - | FileCheck %s

struct pointed_to_t;
typedef pointed_to_t *pointed_to_p;

struct array_to_t {
  pointed_to_p ptr2;
} ;

struct pt_to_base_t {
  int i;
  virtual void foo() {}
};

struct pointed_to_t : pt_to_base_t {
  pointed_to_p my_ptr;
  array_to_t array[5];
};


struct used_base_t {
  int i;
  virtual void foo(){}
};

struct used_t : used_base_t {
  pointed_to_p pointed;
};

used_t used;

struct pt_to_by_func_t {};

void func(pt_to_by_func_t*){};
// CHECK: define {{.*}}void @_Z4funcP15pt_to_by_func_t(ptr noundef "intel_dtrans_func_index"="1" %{{.+}}){{.*}}!intel.dtrans.func.type ![[FUNC_INFO:[0-9]+]]

// CHECK: !intel.dtrans.types = !{![[USED:[0-9]+]], ![[USED_BASE:[0-9]+]], ![[USED_BASE_BASE:[0-9]+]], ![[PT_TO_BY_FUNC:[0-9]+]], ![[PTRED_TO:[0-9]+]], ![[PT_TO_BASE:[0-9]+]], ![[PT_TO_BASE_BASE:[0-9]+]], ![[ARR_TO:[0-9]+]]}

// CHECK: ![[USED]]  = !{!"S", %struct._ZTS6used_t.used_t zeroinitializer, i32 2, ![[USED_BASE_BASE_REF:[0-9]+]], ![[PTRED_TO_PTR:[0-9]+]]}
// CHECK: ![[USED_BASE_BASE_REF]]  = !{%struct._ZTS11used_base_t.used_base_t.base zeroinitializer, i32 0}
// CHECK: ![[PTRED_TO_PTR]]  = !{%struct._ZTS12pointed_to_t.pointed_to_t zeroinitializer, i32 1}
// CHECK: ![[USED_BASE]] = !{!"S", %struct._ZTS11used_base_t.used_base_t zeroinitializer, i32 3, ![[VTABLE_PTR:[0-9]+]], ![[INT:[0-9]+]], ![[PAD_ARR:[0-9]+]]}
// CHECK: ![[VTABLE_PTR]] = !{![[VTABLE:[0-9]+]], i32 2}
// CHECK: ![[VTABLE]] = !{!"F", i1 true, i32 0, ![[INT]]}
// CHECK: ![[INT]]  = !{i32 0, i32 0}
// CHECK: ![[PAD_ARR]] = !{!"A", i32 4, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[USED_BASE_BASE]] = !{!"S", %struct._ZTS11used_base_t.used_base_t.base zeroinitializer, i32 2, ![[VTABLE_PTR]], ![[INT]]}
// CHECK: ![[PT_TO_BY_FUNC]] = !{!"S", %struct._ZTS15pt_to_by_func_t.pt_to_by_func_t zeroinitializer, i32 1, ![[CHAR]]}
// CHECK: ![[PTRED_TO]] = !{!"S", %struct._ZTS12pointed_to_t.pointed_to_t zeroinitializer, i32 3, ![[PT_TO_BASE_BASE_REF:[0-9]+]], ![[PTRED_TO_PTR]], ![[ARR_TO_ARRAY:[0-9]+]]}
// CHECK: ![[PT_TO_BASE_BASE_REF]] = !{%struct._ZTS12pt_to_base_t.pt_to_base_t.base zeroinitializer, i32 0}
// CHECK: ![[ARR_TO_ARRAY]] = !{!"A", i32 5, ![[ARR_TO_REF:[0-9]+]]}
// CHECK: ![[ARR_TO_REF]] = !{%struct._ZTS10array_to_t.array_to_t zeroinitializer, i32 0}
// CHECK: ![[PT_TO_BASE]] = !{!"S", %struct._ZTS12pt_to_base_t.pt_to_base_t zeroinitializer, i32 3, ![[VTABLE_PTR]], ![[INT]], ![[PAD_ARR]]}
// CHECK: ![[PT_TO_BASE_BASE]] = !{!"S", %struct._ZTS12pt_to_base_t.pt_to_base_t.base zeroinitializer, i32 2, ![[VTABLE_PTR]], ![[INT]]}
// CHECK: ![[ARR_TO]] = !{!"S", %struct._ZTS10array_to_t.array_to_t zeroinitializer, i32 1, ![[PTRED_TO_PTR]]}
// CHECK: ![[FUNC_INFO]] = distinct !{![[PT_TO_BY_FUNC_PTR:[0-9]+]]}
// CHECK: ![[PT_TO_BY_FUNC_PTR]] = !{%struct._ZTS15pt_to_by_func_t.pt_to_by_func_t zeroinitializer, i32 1}
