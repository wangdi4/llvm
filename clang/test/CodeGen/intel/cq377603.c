// RUN: %clang_cc1 -O0 -triple x86_64-windows -fintel-compatibility -verify %s -emit-llvm -o - | FileCheck %s

__vectorcall // expected-warning {{vectorcall calling convention is not supported on variadic function}}
void foo(double p1, ...)
{}
// CHECK: define dso_local void @{{.+}}(double noundef {{%.+}}, ...) #0 {
