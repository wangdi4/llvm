// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
_Bool state;

void test(_Bool *in) {
  state = *in;
}

// CHECK: define dso_local void @test(i8* noundef "intel_dtrans_func_index"="1" %{{.*}}){{.*}}!intel.dtrans.func.type ![[TEST:[0-9]+]]
// CHECK: ![[TEST]] = distinct !{![[BOOL_PTR:[0-9]]]}
// CHECK: ![[BOOL_PTR]] = !{i8 0, i32 1}
