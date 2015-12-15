// RUN: %clang_cc1 -triple i686-unknown-linux -fsyntax-only -fintel-compatibility -verify -emit-llvm %s -o - | FileCheck  %s
// RUN: %clang_cc1 -triple i686-pc-windows -fsyntax-only -fintel-compatibility -verify -emit-llvm %s -o - | FileCheck %s
//
// intel-clang should emit a warning, not an error, if function is re-declared
// with a new regparm attribute. CQ#374880.

int foo1(int a, int b, int c); // expected-note {{previous declaration is here}}
int foo2(int a, int b, int c) __attribute__((regparm(2))); // expected-note {{previous declaration is here}}

__attribute__((regparm(1))) int foo1(int a, int b, int c) // expected-warning {{declaration of 'foo1' is incompatible with previous declaration}}
{
  return a + b + c;
}
// CHECK: define {{.*}}foo1{{.*}}i32 inreg %a, i32 %b, i32 %c

__attribute__((regparm(3))) int foo2(int a, int b, int c) // expected-warning {{declaration of 'foo2' is incompatible with previous declaration}}
{
  return a + b - c;
}
// CHECK: define {{.*}}foo2{{.*}}i32 inreg %a, i32 inreg %b, i32 %c

int main() {
  int res = foo1(100, 20, 1) - foo2(100, 20, 1);
  if (res == 2)
    return 0;
  return 1;
}

