// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
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
// CHECK: define dso_local void @test2(i32* noundef "intel_dtrans_func_index"="1" %i) {{.*}}!intel.dtrans.func.type ![[TEST2_FUNC_MD:[0-9]+]]
void test2(int *i) {
  // CHECK: alloca i32*, align 8, !intel_dtrans_type ![[INT_PTR:[0-9]+]]
}

// CHECK: define dso_local "intel_dtrans_func_index"="1" i32* @test3(i32* noundef "intel_dtrans_func_index"="2" %i) {{.*}}!intel.dtrans.func.type ![[TEST3_FUNC_MD:[0-9]+]]
int *test3(int *i) {
  // CHECK: alloca i32*, align 8, !intel_dtrans_type ![[INT_PTR]]
  return i;
}

// CHECK: ![[INT_PTR]] = !{i32 0, i32 1}
// CHECK: ![[TEST2_FUNC_MD]] = distinct !{![[INT_PTR]]}
// CHECK: ![[TEST3_FUNC_MD]] = distinct !{![[INT_PTR]], ![[INT_PTR]]}
