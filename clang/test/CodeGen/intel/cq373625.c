// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility %s -opaque-pointers -emit-llvm -o - | FileCheck %s

char buf[10];
// CHECK: [[BUF:@.+]] = {{.*}}global [10 x i8] zeroinitializer
// CHECK: [[FMT:@.+]] = private unnamed_addr constant [9 x i8] c"foo: %d\0A\00"
// CHECK-LABEL: @foo(
int foo(int a) {
// CHECK: [[A_VAL:%.+]] = load i32, ptr %
// CHECK: call i32 (ptr, ptr, ...) @sprintf(ptr noundef [[BUF]], ptr noundef [[FMT]], i32 noundef [[A_VAL]])
  return __builtin_sprintf(buf, "foo: %d\n", a);
}
