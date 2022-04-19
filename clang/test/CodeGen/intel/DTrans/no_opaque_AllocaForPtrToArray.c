// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,PTRS
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ

// PTRS: define dso_local void @test(i8* noundef "intel_dtrans_func_index"="1" %buf) {{.*}}!intel.dtrans.func.type ![[TEST_MD:[0-9]+]]
// OPQ: define dso_local void @test(ptr noundef "intel_dtrans_func_index"="1" %buf) {{.*}}!intel.dtrans.func.type ![[TEST_MD:[0-9]+]]
void test(void *buf) {
  // PTRS: alloca i8*, align 8, !intel_dtrans_type ![[VOID_PTR:[0-9]+]]
  // OPQ: alloca ptr, align 8, !intel_dtrans_type ![[VOID_PTR:[0-9]+]]
  // PTRS: alloca [4 x i32]*, align 8, !intel_dtrans_type ![[ARRAY_PTR:[0-9]+]]
  // OPQ: alloca ptr, align 8, !intel_dtrans_type ![[ARRAY_PTR:[0-9]+]]
  int (*sum0)[4] = buf;
  }

int main() {
    return 0;
}

// CHECK: ![[TEST_MD]] = distinct !{![[VOID_PTR]]}
// CHECK: ![[VOID_PTR]] = !{i8 0, i32 1}
// CHECK: ![[ARRAY_PTR]] = !{![[ARRAY_TY:[0-9]+]], i32 1}
// CHECK: ![[ARRAY_TY]] = !{!"A", i32 4, ![[INT_TY:[0-9]+]]}
// CHECK: ![[INT_TY]] = !{i32 0, i32 0}
