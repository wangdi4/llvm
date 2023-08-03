// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,LIN
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,WIN

struct bitmap_head {
  constexpr bitmap_head()
    : padding (0), alloc_descriptor (0), obstack (&crashme)
  {}

  static int crashme;

  unsigned padding: 1;
  unsigned alloc_descriptor: 25;
  void * obstack;
};


static bitmap_head try_hard_reg_pseudos[1];
void foo () {
  (void)try_hard_reg_pseudos;
}


// LIN: @_ZL20try_hard_reg_pseudos = internal global [1 x { i8, i8, i8, i8, ptr }]{{.*}} !intel_dtrans_type ![[TYPE:[0-9]+]]
// WIN: @try_hard_reg_pseudos = internal global [1 x { i8, i8, i8, i8, ptr }]{{.*}} !intel_dtrans_type ![[TYPE:[0-9]+]]
//
// CHECK: ![[TYPE]] = !{!"A", i32 1, ![[LITERAL_REF:[0-9]+]]}
// CHECK: ![[LITERAL_REF]] = !{![[LITERAL:[0-9]+]], i32 0}
// CHECK: ![[LITERAL]] = !{!"L", i32 5, ![[CHAR:[0-9]+]], ![[CHAR]], ![[CHAR]], ![[CHAR]], ![[CHAR_PTR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[CHAR_PTR]] = !{i8 0, i32 1}

