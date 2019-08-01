// RUN: %clang_cc1 -fintel-compatibility -verify -emit-llvm -o - %s

void bar() {
  struct foo d[];   // expected-error {{definition of variable with array type needs an explicit size or an initializer}} \
                  // expected-note {{forward declaration}}
  struct foo c[10]; // expected-error {{variable has incomplete type 'struct foo'}}
}
