// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-pc-windows-msvc -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s

// Validate the the windows VTable (as checked) has attached metadata.
struct Base {
  virtual void func();
};

struct Inherits : Base {
  Inherits();
  void func();
};

Inherits::Inherits() {}

// CHECK: @{{[0-9]+}} = private unnamed_addr constant { [2 x ptr] } { [2 x ptr] {{.*}} }, comdat{{.*}}, !intel_dtrans_type ![[VTABL:[0-9]+]]

// CHECK: ![[VTABL]] = !{!"L", i32 1, ![[ARR:[0-9]+]]}
// CHECK: ![[ARR]] = !{!"A", i32 2, ![[CHARPTR:[0-9]+]]}
// CHECK: ![[CHARPTR]] = !{i8 0, i32 1}
