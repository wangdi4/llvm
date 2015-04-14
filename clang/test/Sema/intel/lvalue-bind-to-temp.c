// CQ#364712
// RUN: %clang_cc1 -fintel-ms-compatibility -x c++ -verify %s -DWARNING
// RUN: %clang_cc1 -fintel-ms-compatibility -x c++ -std=c++11 -verify %s -DCXX11WARNING
// RUN: %clang_cc1 -x c++ -verify %s -DERROR
// RUN: %clang_cc1 -x c++ -std=c++11 -verify %s -DCXX11ERROR
// RUN: %clang_cc1 -x c++ -std=c++11 -verify %s -DOK
// RUN: %clang_cc1 -fintel-ms-compatibility -x c++ -std=c++11 -DOK

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
  C &c = (argc > 2 ? C(55) : C(42)); // expected-warning {{non-const lvalue reference to type 'C' cannot bind to a temporary of type 'C'}}
#elif CXX11WARNING
  C &c{ C(55) };  // expected-warning {{non-const lvalue reference to type 'C' cannot bind to an initializer list temporary}}
  D d{ 1, { 2, 3 } }; // expected-warning {{non-const lvalue reference to type 'int [2]' cannot bind to an initializer list temporary}}
#elif ERROR
  C &c = (argc > 2 ? C(55) : C(42)); // expected-error {{non-const lvalue reference to type 'C' cannot bind to a temporary of type 'C'}}
#elif CXX11ERROR
  C &c{ C(55) }; // expected-error {{non-const lvalue reference to type 'C' cannot bind to an initializer list temporary}}
  D d{ 1, { 2, 3 } }; // expected-error {{non-const lvalue reference to type 'int [2]' cannot bind to an initializer list temporary}}
  volatile C &c2{ C(10) }; // expected-error {{volatile lvalue reference to type 'volatile C' cannot bind to an initializer list temporary}}
#elif OK
  const C &c = (argc > 2 ? C(55) : C(42)); // expected-no-diagnostics
  const C &c2{ C(55) }; // expected-no-diagnostics
#else

#error Unknown test mode

#endif
}
