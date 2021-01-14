// RUN: %clang_cc1 -triple x86_64-windows-msvc -fintel-compatibility \
// RUN:            -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-windows-msvc -fintel-compatibility-enable=PragmaInline \
// RUN:            -emit-llvm -o - %s | FileCheck %s

// RUN: %clang_cc1 -triple x86_64-windows-msvc -fintel-compatibility \
// RUN:            -fintel-compatibility-disable=PragmaInline -Wunknown-pragmas \
// RUN:            -verify %s
// RUN: %clang_cc1 -triple x86_64-windows-msvc -fintel-compatibility-disable=PragmaInline \
// RUN:            -Wunknown-pragmas -verify %s

// RUN: %clang_cc1 -triple x86_64-windows-msvc -verify -Wunknown-pragmas %s

void foo() {}

void bar() {
#pragma forceinline recursive // expected-warning {{unknown pragma ignored}}
  //CHECK call void @"?foo@@YAXXZ"() #1
  foo();
}

void baz() {
#pragma noinline // expected-warning {{unknown pragma ignored}}
  //CHECK: call void @"?foo@@YAXXZ"() #2
  foo();
}

void bat() {
#pragma inline // expected-warning {{unknown pragma ignored}}
  // CHECK:   call void @"?foo@@YAXXZ"() #3
  foo();
}

void ba1() {
#pragma inline recursive // expected-warning {{unknown pragma ignored}}
  //CHECK: call void @"?foo@@YAXXZ"() #4
  foo();
}

// CHECK: attributes #1 = { "always-inline-recursive" }
// CHECK: attributes #2 = { noinline }
// CHECK: attributes #3 = { inlinehint }
// CHECK: attributes #4 = { "inline-hint-recursive" }
