// RUN: %clang_cc1 -emit-llvm %s -o - -fms-compatibility -triple=x86_64-apple-darwin12 | FileCheck %s
// RUN: %clang_cc1 -emit-llvm %s -o - -fms-compatibility -triple=x86_64-pc-win32 -fms-compatibility | FileCheck %s -check-prefix CHECK-MS

// CHECK: [[GLOB_A:@.+]] = global i32 0
// CHECK-MS: [[GLOB_A:@.+]] = global i32 0
int a;

// CHECK: define{{.*}} i32 [[FOO:@.+foo.+]](i32*
// CHECK-MS: define{{.*}} i32 [[FOO:@.+foo.+]](i32*
int foo(int &a, int &b = a, int &c = a) {
  return a + b + c;
}

// CHECK-LABEL: @main
// CHECK-MS-LABEL: @main
int main() {
  // CHECK:   {{%.+}} = call{{.*}} i32 [[FOO]](i32* {{.*}}[[GLOB_A]], i32* {{.*}}[[GLOB_A]], i32* {{.*}}[[GLOB_A]])
  // CHECK-MS: [[REF_PARM:%.+]] = alloca i32*
  // CHECK-MS: store i32* [[GLOB_A]], i32** [[REF_PARM]]
  // CHECK-MS: [[PARM:%.+]] = load i32** [[REF_PARM]]
  // CHECK-MS: {{%.+}} = call{{.*}} i32 [[FOO]](i32* {{.*}}[[PARM]], i32* {{.*}}[[PARM]], i32* {{.*}}[[PARM]])
  return foo(a);
}

