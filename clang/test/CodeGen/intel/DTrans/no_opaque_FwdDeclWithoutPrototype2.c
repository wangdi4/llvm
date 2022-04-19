// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
// Same as FwdDeclWithoutPrototype.c, except we have definitions.

void test();
void test2();
int *test3();

int main() {
  test();
  int i;
  test2(&i);
  int *j = test3(i);
  return 0;
}

void test() {
}
// PTR: define dso_local void @test2(i32* noundef "intel_dtrans_func_index"="1" %i) {{.*}}!intel.dtrans.func.type ![[TEST2_FUNC_MD:[0-9]+]]
// OPQ: define dso_local void @test2(ptr noundef "intel_dtrans_func_index"="1" %i) {{.*}}!intel.dtrans.func.type ![[TEST2_FUNC_MD:[0-9]+]]
void test2(int *i) {
  // PTR: alloca i32*, align 8, !intel_dtrans_type ![[INT_PTR:[0-9]+]]
  // OPQ: alloca ptr, align 8, !intel_dtrans_type ![[INT_PTR:[0-9]+]]
}

// PTR: define dso_local "intel_dtrans_func_index"="1" i32* @test3(i32* noundef "intel_dtrans_func_index"="2" %i) {{.*}}!intel.dtrans.func.type ![[TEST3_FUNC_MD:[0-9]+]]
// OPQ: define dso_local "intel_dtrans_func_index"="1" ptr @test3(ptr noundef "intel_dtrans_func_index"="2" %i) {{.*}}!intel.dtrans.func.type ![[TEST3_FUNC_MD:[0-9]+]]
int *test3(int *i) {
  // PTR: alloca i32*, align 8, !intel_dtrans_type ![[INT_PTR]]
  // OPQ: alloca ptr, align 8, !intel_dtrans_type ![[INT_PTR]]
  return i;
}

// CHECK: ![[INT_PTR]] = !{i32 0, i32 1}
// CHECK: ![[TEST2_FUNC_MD]] = distinct !{![[INT_PTR]]}
// CHECK: ![[TEST3_FUNC_MD]] = distinct !{![[INT_PTR]], ![[INT_PTR]]}
