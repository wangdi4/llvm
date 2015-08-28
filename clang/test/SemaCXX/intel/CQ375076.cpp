// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

class C {
public:
  // expected-note@+1 {{candidate constructor}}
  C() {}
  // expected-note@+1 {{previous declaration is here}}
  C(bool flag);
};

// expected-note@+2 {{candidate constructor}}
// expected-warning@+1 {{addition of default argument on redeclaration makes this constructor a default constructor}}
C::C(bool flag = false) {}

int main() {
  // expected-error@+1 {{call to constructor of 'C' is ambiguous}}
  C c;
  C c1(false);
  return 0;
}
