// RUN: %clang_cc1 -triple x86_64-apple-darwin10.0.0 -emit-llvm -o - %s -fexceptions -std=c++11 -g -fintel-compatibility | FileCheck %s

// CHECK-LABEL: foo
int foo(int x) {
// CHECK: [[X:%.+]] = alloca i32,
// CHECK: call void @llvm.dbg.declare(
// CHECK: [[X_REF:%.+]] = getelementptr inbounds %{{.+}}, %{{.+}}* %{{.+}}, i32 0, i32 0, !dbg ![[DBG_FOO:[0-9]+]]
// CHECK: [[X_VAL:%.+]] = load i32, i32* [[X]], align 4, !dbg ![[DBG_FOO]]
// CHECK: store i32 [[X_VAL]], i32* [[X_REF]], align 4, !dbg ![[DBG_FOO]]
// CHECK: call i32 @{{.+}}, !dbg ![[DBG_FOO]]
  return [=] {
    return x;
  }();
}

// CHECK: [[DBG_FOO:![0-9]+]] = !DILocation(line: 11,
