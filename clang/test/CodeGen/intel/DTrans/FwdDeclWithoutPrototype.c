// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

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

// CHECK: declare !intel.dtrans.func.type ![[TEST3_FUNC_MD:[0-9]+]] "intel_dtrans_func_index"="1" i32* @test3(...)
// CHECK: ![[INT_PTR:[0-9]+]] = !{i32 0, i32 1}
// CHECK: ![[TEST3_FUNC_MD]] = distinct !{![[INT_PTR]]}
