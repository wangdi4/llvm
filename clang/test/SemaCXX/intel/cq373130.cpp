// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s
// expected-no-diagnostics

struct S {
  static const int i = 3;
  friend int f(int = i);
};

int f(int c) {
  c++;
  return c;
}

int main() {
  return f();
}

