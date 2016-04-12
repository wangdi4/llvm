// RUN: %clang_cc1 %s -fexceptions -O0 -fintel-compatibility -verify

namespace member_pointers {
struct S {
  template <typename T>
  bool f(T) { return false; }
  template <typename T>
  static bool g(T) { return false; }

  template <typename T>
  bool h(T) { return false; } // expected-note 3 {{possible target for call}}
  template <int N>
  static bool h(int) { return false; } // expected-note 3 {{possible target for call}}
};

void test(S s) {
  if (&S::f<char>)
    return;
  if (&S::f<int>)
    return;
  if (&s.f<char>) // expected-error {{cannot create a non-constant pointer to member function}}
    return;
  if (&s.f<int>) // expected-error {{cannot create a non-constant pointer to member function}}
    return;

  if (&S::g<char>)
    return;
  if (&S::g<int>)
    return;
  if (&s.g<char>)
    return;
  if (&s.g<int>)
    return;

  if (S::h<42>) // expected-warning {{address of function 'S::h<42>' will always evaluate to 'true'}} \
                // expected-note {{prefix with the address-of operator to silence this warning}}
    return;

  if (S::h<int>) // expected-error {{reference to overloaded function could not be resolved; did you mean to call it?}}
    return;
  if (&S::h<42>)
    return;
  if (&S::h<int>)
    return;
  if (s.h<int>) // expected-error {{reference to overloaded function could not be resolved; did you mean to call it?}}
    return;
  if (&s.h<42>)
    return;
  if (&s.h<int>) // expected-error {{reference to overloaded function could not be resolved; did you mean to call it?}}
    return;

  { bool b = &S::f<char>; }
  { bool b = &S::f<int>; }
  { bool b = &s.f<char>; } // expected-error {{cannot create a non-constant pointer to member function}}
  { bool b = &s.f<int>; }  // expected-error {{cannot create a non-constant pointer to member function}}
}
}
