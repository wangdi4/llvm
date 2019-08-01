// RUN: %clang_cc1 %s -fsyntax-only -fintel-compatibility -verify

// intel-clang emits a warning, not an error, in case of function's return type
// specifier missing in a C++ program (standard clang emits a warning for C, but
// an error for C++). CQ#364053.
foo() { // expected-warning {{type specifier missing, defaults to 'int'}}
  return 0;
}

