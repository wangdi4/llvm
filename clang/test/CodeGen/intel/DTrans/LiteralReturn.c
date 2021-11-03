// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
struct {
  int a : 16;
  char *b;
} b[] = {1, (char *)0};

// CHECK: @b = global [1 x { i8, i8, i8* }] [{ i8, i8, i8* } { i8 1, i8 0, i8* null }], align 16, !intel_dtrans_type ![[B:[0-9]+]]
// CHECK: !intel.dtrans.types = !{}

// CHECK: ![[B]] = !{!"A", i32 1, ![[LITERAL_REF:[0-9]+]]}
// CHECK: ![[LITERAL_REF]] = !{![[LITERAL:[0-9]+]], i32 0}
// CHECK: ![[LITERAL]] = !{!"L", i32 3, ![[CHAR:[0-9]+]], ![[CHAR]], ![[CHAR_PTR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[CHAR_PTR]] = !{i8 0, i32 1}
