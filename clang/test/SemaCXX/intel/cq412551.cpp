// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility -gnu-permissive -triple x86_64-unknown-linux-gnu %s
// expected-no-diagnostics

class A {
  operator int*();
public:
  operator int() const;
};

int foo() {
  return A();
}
