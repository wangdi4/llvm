// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s
// expected-no-diagnostics

int i;
struct C {
  C();
};

C::C() {
  static void *labelref = &&label;
  goto *labelref;
label:
  i = 1;
}

int main() {
  C c;
  return (i != 1);
}
