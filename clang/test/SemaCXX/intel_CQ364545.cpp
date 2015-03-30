// RUN: %clang_cc1 -fms-compatibility -verify -fsyntax-only %s
template <class T>
T foo(T &a, T &b = a, T &c = a) {
  return a + b + c;
}

int bar(int &a, int &b = a, int &c = a) { // expected-error 2 {{default argument references parameter 'a'}}
  return a + b + c;
}
