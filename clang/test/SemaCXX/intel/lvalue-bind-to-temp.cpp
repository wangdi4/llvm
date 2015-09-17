// CQ#364712
// RUN: %clang_cc1 -x c++ -std=c++11 -fintel-ms-compatibility -fsyntax-only -O0 -verify %s -DWARNING
// RUN: %clang_cc1 -x c++ -std=c++11 -fintel-ms-compatibility -fsyntax-only -O0 -verify %s -DERROR
// RUN: %clang_cc1 -x c++ -std=c++11 -fintel-ms-compatibility -fsyntax-only -DOK

struct C {
  int i;
  C(int ii) : i(ii) {}
};

struct D {
  int a;
  int (&b)[2];
};

int main(int argc, char **argv) {
#if WARNING
  C &c1{ C(10) }; // expected-warning {{non-const lvalue reference to type 'C' cannot bind to an initializer list temporary}}
  C &c2 = C(20); // expected-warning {{non-const lvalue reference to type 'C' cannot bind to a temporary of type 'C'}}
#elif ERROR
  int &x = 1 ? 2 : 3; // expected-error {{non-const lvalue reference to type 'int' cannot bind to a temporary of type 'int'}}
  D d{ 1, { 2, 3 } }; // expected-error {{non-const lvalue reference to type 'int [2]' cannot bind to an initializer list temporary}}
  volatile C &c2{ C(10) }; // expected-error {{volatile lvalue reference to type 'volatile C' cannot bind to an initializer list temporary}}
#elif OK
  // expected-no-diagnostics
  const int &x = 1 ? 2 : 3;
  const C &c = (argc > 2 ? C(55) : C(42));
  const C &c2{ C(55) };
#else

#error Unknown test mode

#endif
}
