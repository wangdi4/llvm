// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility %s

template <class T> void foo (int);

template <class T>
class Q {
  friend void foo<T> (int = 3); // expected-error {{friend declaration specifying a default argument must be a definition}}
};

template <int N>
struct A {
  template<int M> void B () ;
};

void A<0>::B<0>() {    // expected-error {{template specialization requires 'template<>'}}
}

