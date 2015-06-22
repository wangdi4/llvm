// CQ#371284
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

void foo(int x);

void test() {
  // CHECK: %{{.+}} = alloca i32
  // CHECK-NEXT: %{{.+}} = ptrtoint i32* %{{.+}} to i32
  // CHECK-NEXT: call void @foo(i32 %{{.+}})
  int a __attribute__((cleanup(foo)));
}
