// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility %s

template <class T> struct CI {
  CI();
  CI(int i);
};

struct CI1 {
  int j;
  CI1(int j) : j(j) {}
};

template <int i> struct CI2 {
  CI2();
};

template <class T> struct CDI : public CI<T>, public CI1 {
  CDI() : CI(2), CI1(3) {} // expected-warning{{missing parameter for template class 'CI'}}
};

template <class T> class CDJ : public CI<T>, public CI1 {
  int CI;
public:
  CDJ() : CI(4), CI1(5) {} // not-expected-warning
};

class CDK : public CI<float> {
  CDK(int i) : CDK::CI(i) {} // not-expected-warning
};

class CDL : public CI2<5> {
  CDL() : CI2() {} // not-expected-warning
};

template<class T>
struct A {
      A(T);
};

template<class T1, class T2>
struct B : A<T1> {
      B(T1 t) : A(t) {} // expected-warning{{missing parameter for template class 'A'}}:
};

B<int, int> b(0);

template<int i>
struct A1 {
    A1(int);
};

template<int i>
struct B1 : A1<i> {
    B1(int t) : A1(t) {} // expected-warning{{missing parameter for template class 'A1'}}
};

B1<1> b1(0);

