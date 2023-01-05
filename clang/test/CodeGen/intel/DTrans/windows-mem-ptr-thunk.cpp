// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows-pc -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple i386-windows-pc -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s

// Ensure that we properly emit the dtrans information for a mem-ptr thunk.
// Previously in CMPLRLLVM-41283 we did this for tail-call thunks.

struct FormatterListener
{ virtual double characters(float, double, FormatterListener&) = 0; };

void use()
{ (void)&FormatterListener::characters; }

// Note: Wildcard needed in function name since the mangling between 32 and 64
// bit windows is slightly different.
// CHECK: define {{.*}}void @"??_9FormatterListener@@$BA@A{{.}}"(ptr noundef "intel_dtrans_func_index"="1" %this, ...){{.*}}!intel.dtrans.func.type ![[THUNK:[0-9]+]]

// CHECK: ![[THUNK]] = distinct !{![[STRUCT_PTR:[0-9]+]]}
// CHECK: ![[STRUCT_PTR]] = !{%"struct..?AUFormatterListener@@.FormatterListener" zeroinitializer, i32 1}
