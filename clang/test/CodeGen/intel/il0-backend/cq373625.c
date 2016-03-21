// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility %s -emit-llvm -o - | FileCheck %s

char buf[10];
// CHECK: [[BUF:@.+]] = common global [10 x i8] zeroinitializer
// CHECK: [[FMT:@.+]] = private unnamed_addr constant [9 x i8] c"foo: %d\0A\00"
// CHECK-LABEL: @foo(
int foo(int a) {
// CHECK: [[A_VAL:%.+]] = load i32, i32* %
// CHECK: call i32 (i8*, i8*, ...) @__builtin_sprintf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* [[BUF]], i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* [[FMT]], i32 0, i32 0), i32 [[A_VAL]])
  return __builtin_sprintf(buf, "foo: %d\n", a);
}
