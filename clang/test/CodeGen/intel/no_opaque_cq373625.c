// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility %s -emit-llvm -no-opaque-pointers -o - | FileCheck %s

char buf[10];
// CHECK: [[BUF:@.+]] = {{.*}}global [10 x i8] zeroinitializer
// CHECK: [[FMT:@.+]] = private unnamed_addr constant [9 x i8] c"foo: %d\0A\00"
// CHECK-LABEL: @foo(
int foo(int a) {
// CHECK: [[A_VAL:%.+]] = load i32, i32* %
// CHECK: call i32 (i8*, i8*, ...) @sprintf(i8* noundef getelementptr inbounds ([10 x i8], [10 x i8]* [[BUF]], i64 0, i64 0), i8* noundef getelementptr inbounds ([9 x i8], [9 x i8]* [[FMT]], i64 0, i64 0), i32 noundef [[A_VAL]])
  return __builtin_sprintf(buf, "foo: %d\n", a);
}
