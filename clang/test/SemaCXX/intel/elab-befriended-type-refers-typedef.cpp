// CQ#364598
// RUN: %clang_cc1 -std=c++11 -fsyntax-only -fintel-compatibility -verify %s

struct Cake1 {
  bool is_a_lie();
};

struct Cake2 {
  bool is_a_lie();
};

typedef Cake2 Cake2Typedefed; // expected-note{{declared here}}

class B {
public:
  typedef Cake1 Cake1Typedefed; // expected-note{{declared here}}
  friend struct Cake1Typedefed; // expected-warning{{elaborated type refers to a typedef}}
  friend struct Cake2Typedefed; // expected-error{{elaborated type refers to a typedef}}
};

bool B::Cake1Typedefed::is_a_lie() {
  return true;
}

bool Cake2Typedefed::is_a_lie() {
  return true;
}

// CQ#374878
struct C {};
struct D {
  typedef struct {} ValueType2; // expected-note2{{declared here}}
};

template <class T>
struct E {
  typedef T ValueType1; // expected-note2{{declared here}}
};

template <class T>
struct Test1 {
  friend class E<T>::ValueType1; // expected-warning{{elaborated type refers to a typedef}}
                                 // expected-error@-1{{elaborated type refers to a typedef}}
};

template <class T>
struct Test2 {
  typedef struct T::ValueType2 TestType2; // expected-warning{{elaborated type refers to a typedef}}
  using TestType3 = struct T::ValueType2; // expected-warning{{elaborated type refers to a typedef}}
};

int main() {
  B::Cake1Typedefed cake1;
  Cake2Typedefed cake2;

  Test1<C>    test1; // expected-note{{in instantiation of}}
  Test1<int>  test2; // expected-note{{in instantiation of}}
  Test2<D>    test3; // expected-note{{in instantiation of}}

  return cake1.is_a_lie() && cake2.is_a_lie();
}
