// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -emit-llvm %s -opaque-pointers -o - | FileCheck %s

struct S {int a,b;};
void bar(S);
 
void foo() {
    bar({1,2});
// CHECK: [[alloca:%.+]] = alloca %struct.S
// CHECK: [[t2:%.+]] = load i64, ptr [[alloca]], align 4
// CHECK: call void @_Z3bar1S(i64 [[t2]])
}
