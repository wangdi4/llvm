// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility %s

#define size2 5
template <typename T, int size1>
struct A {};

template <typename T, int size>
struct B {
  template <typename U, int size3>
  void foo(A<T, size2> i);
};

template <>
void B<char, 4>::foo<int, 5>(A<char, 5> i) { ; } // expected-error {{template specialization requires 'template<>'}}

void foo() {
  A<char, size2> i;
  B<char, 4> b;
  // i == i;
  b.foo<int, size2>(i);
}

