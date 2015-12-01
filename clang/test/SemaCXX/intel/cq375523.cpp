// RUN: %clang_cc1 -fintel-compatibility -verify %s

struct S {
  static int f() { return 1; }
};

struct A {
  template<class B> int f() {
    return B::f();
  };
};

template<class T, class B>
int g(T *p) {
  return p->f<B>(); // expected-warning{{use 'template' keyword to treat 'f' as a dependent template name}}
}

int main() {
  A a;
  int x1 = g<A, S>(&a);
  return 0;
}
