// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s

struct S {int a,b;};
void bar(S);
 
void foo() {
    bar({1,2});
// CHECK: [[t1:%.+]] = bitcast %struct.S* {{%.+}} to i64*
// CHECK: [[t2:%.+]] = load i64, i64* [[t1]], align 4
// CHECK: call void @_Z3bar1S(i64 [[t2]])
}
