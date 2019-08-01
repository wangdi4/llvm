// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

class Test {
  virtual void foo1() = 0;
  virtual void foo2() = 0L; // expected-warning{{initializer on function does not look like a pure-specifier}}
};
