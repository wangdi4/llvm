// RUN: %clang-cc1 -triple x86_64-unknown-unknown -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

void *p;

void foo(void) {
  p = _alloca(16);
  // CHECK: %{{.+}} = alloca i8, i64 16
  *((int*)p) = 1;
  // CHECK: store i32 1, i32* %{{.+}}, align 4
}
