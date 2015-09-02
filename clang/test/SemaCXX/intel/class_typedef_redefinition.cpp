// CQ#373258
// RUN: %clang_cc1 -std=c++11 -fintel-compatibility -fsyntax-only -verify %s

template <class T> struct check_int_ptr_type {
  static const bool value = false;
};

template <> struct check_int_ptr_type<int *> {
  static const bool value = true;
};

class A {
  typedef int *int_pointer; // expected-note{{previous definition is here}}
  typedef int *int_pointer; // expected-warning{{class member typedef redefinition is an Intel extension}}

  static_assert(check_int_ptr_type<int_pointer>::value,
                "int_pointer is not int *");
};
