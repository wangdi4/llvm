// RUN: %clang_cc1 -fintel-compatibility -verify %s

template <class T>
class C {
public:
  void mf1(T p);
  void mf2(T p);
};

template <class T>
void C<T>::mf1(T p) { return; }
template <class T>
void C<T>::mf2(T p) { return 0; } // expected-warning{{void function 'mf2' should not return a value}}

int foo() {
  C<int> c;
  c.mf1(10);
  return 0;
}
