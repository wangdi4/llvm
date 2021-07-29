// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

// CHECK: define dso_local void @test(i8* "intel_dtrans_func_index"="1" %buf) {{.*}}!intel.dtrans.func.type ![[TEST_MD:[0-9]+]]
void test(void *buf) {
  // CHECK: alloca i8*, align 8, !intel_dtrans_type ![[VOID_PTR:[0-9]+]]
  // CHECK: alloca [4 x i32]*, align 8, !intel_dtrans_type ![[ARRAY_PTR:[0-9]+]]
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
