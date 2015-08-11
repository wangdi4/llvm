// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility %s

// Make sure we accept this
template<class X>struct A{typedef X Y;};
template<class X>bool operator==(A<X>,typename A<X>::Y); // expected-note{{candidate template ignored: could not match 'A<type-parameter-0-0>' against 'B<int> *'}}

int a(A<int> x) { return operator==(x,1); }

int a0(A<int> x) { return x == 1; }

template<class X>struct B{typedef X Y;};
template<class X>bool operator==(B<X>*,typename B<X>::Y); // expected-note{{candidate template ignored: substitution failure [with X = int]}}
int a(B<int> x) { return operator==(&x,1); } // expected-error{{no matching function for call to 'operator=='}}

class C {
public:
  int a;
};

template <class T> int operator+(T x, int y) { return x.a + y; }
template <class E> int operator+(int x, E y) { return x + y.a; }

int main(void) {
  C v;
  return (1 + v) + 1;
}

