// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s

void *p;

void foo(void) {
  p = _alloca(16);
  // CHECK: %{{.+}} = alloca i8, i64 16
  *((int*)p) = 1;
  // CHECK: store i32 1, ptr %{{.+}}, align 4
}
