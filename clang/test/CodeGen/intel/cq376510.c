// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -emit-llvm -o - | FileCheck %s
//
// intel-clang emits a warning, not an error, if function declaration has a
// parameter list without types. CQ#376510.
//
int foo(a); // expected-warning {{a parameter list without types is only allowed in a function definition}}

int bar(int b) { return foo(b); }

// CHECK: declare i32 @foo(...)
