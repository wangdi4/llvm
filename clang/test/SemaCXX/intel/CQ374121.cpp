// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

template <typename T>
struct test {
  static const bool result = false;
};

template <typename T>
struct test2 {
  static const bool result = false;
};

template <typename T>
struct my_test {
  static const bool test = test<T>::result;
  static const bool test1 = ::my_test<T>::test1;
  static const bool explicit_global = ::test<T>::result;
  static const bool test2_different_field_name = test2<T>::result;
  int assert1[test ? 1 : -1]; // expected-error {{'assert1' declared as an array with a negative size}}
  int assert2[explicit_global ? 1 : -1]; // expected-error {{'assert2' declared as an array with a negative size}}
};

my_test<int> a; // expected-note {{in instantiation of template class 'my_test<int>' requested here}}
