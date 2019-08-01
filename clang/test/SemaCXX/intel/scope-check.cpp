// RUN: %clang_cc1 -triple x86_64-windows -fsyntax-only -verify -fblocks -fcxx-exceptions -fms-extensions -fintel-compatibility -Wno-unreachable-code %s
// RUN: %clang_cc1 -triple x86_64-windows -fsyntax-only -verify -fblocks -fcxx-exceptions -fms-extensions -fintel-compatibility -std=gnu++11 -Wno-unreachable-code %s

namespace testInvalid {
Invalid inv; // expected-error {{unknown type name}}
// Make sure this doesn't assert.
void fn()
{
    int c = 0;
    if (inv)
Here: ;
    goto Here;
}
}

namespace test0 {
  struct D { ~D(); };

  int f(bool b) {
    if (b) {
      D d;
      goto end;
    }

  end:
    return 1;
  }
}

namespace test1 {
  struct C { C(); };

  int f(bool b) {
    if (b)
      goto foo; // expected-warning {{jump from this goto statement to its label is a Microsoft extension}}
    C c; // expected-note {{jump bypasses variable initialization}}
  foo:
    return 1;
  }
}

namespace test2 {
  struct C { C(); };

  int f(void **ip) {
    static void *ips[] = { &&lbl1, &&lbl2 };

    C c;
    goto *ip;
  lbl1:
    return 0;
  lbl2:
    return 1;
  }
}

namespace test3 {
  struct C { C(); };

  int f(void **ip) {
    static void *ips[] = { &&lbl1, &&lbl2 };

    goto *ip;
  lbl1: {
    C c;
    return 0;
  }
  lbl2:
    return 1;
  }
}

int test3a() {
  goto L; // expected-warning {{jump from this goto statement to its label is a Microsoft extension}}
int a = 5; // expected-note {{jump bypasses variable initialization}}
L:
  return a;
}

namespace test4 {
  struct C { C(); };
  struct D { ~D(); };

  int f(void **ip) {
    static void *ips[] = { &&lbl1, &&lbl2 };

    C c0;

    goto *ip; // expected-warning {{cannot jump}}
    C c1; // expected-note {{jump bypasses variable initialization}}
  lbl1: // expected-note {{possible target of indirect goto}}
    return 0;
  lbl2:
    return 1;
  }
}

namespace test5 {
  struct C { C(); };
  struct D { ~D(); };

  int f(void **ip) {
    static void *ips[] = { &&lbl1, &&lbl2 };
    C c0;

    goto *ip;
  lbl1: // expected-note {{possible target of indirect goto statement}}
    return 0;
  lbl2:
    if (ip[1]) {
      D d; // expected-note {{jump exits scope of variable with non-trivial destructor}}
      ip += 2;
      goto *ip; // expected-warning {{cannot jump from this indirect goto statement to one of its possible targets}}
    }
    return 1;
  }
}

namespace test8 {
  void test1(int c) {
    switch (c) {
    case 0:
      int x = 56; // expected-note {{jump bypasses variable initialization}}
    case 1:       // expected-warning {{jump from switch statement to this case label bypasses variable initialization}}
      x = 10;
    }
  }

  void test2() {
    goto l2;     // expected-warning {{jump from this goto statement to its label is a Microsoft extension}}
  l1: int x = 5; // expected-note {{jump bypasses variable initialization}}
  l2: x++;
  }
}

namespace test10 {
  int test() {
    static void *ps[] = { &&a0 };
    goto *&&a0; // expected-warning {{jump from this goto statement to its label is a Microsoft extension}}
    int a = 3; // expected-note {{jump bypasses variable initialization}}
  a0:
    return 0;
  }
}

namespace test11 {
  struct C {
    C(int x);
    ~C();
  };
  void f(void **ip) {
    static void *ips[] = { &&l0 };
  l0:  // expected-note {{possible target of indirect goto statement}}
    C c0 = 42; // expected-note {{jump exits scope of variable with non-trivial destructor}}
    goto *ip; // expected-warning {{cannot jump from this indirect goto statement to one of its possible targets}}
  }
}

namespace test13 {
  struct C {
    C(int x);
    ~C();
    int i;
  };
  void f(void **ip) {
    static void *ips[] = { &&l0 };
  l0: // expected-note {{possible target of indirect goto}}
    const int &c1 = C(1).i; // expected-note {{jump exits scope of lifetime-extended temporary with non-trivial destructor}}
    goto *ip;  // expected-warning {{cannot jump}}
  }
}

namespace test16 {
  struct S { int n; };
  int f() {
    goto x; // expected-warning {{jump from this goto statement to its label is a Microsoft extension}}
    const S &s = S(); // expected-note {{jump bypasses variable initialization}}
x:  return s.n;
  }
}

#if __cplusplus >= 201103L
namespace test17 {
  struct S { int get(); private: int n; };
  int f() {
    goto x; // expected-warning {{jump from this goto statement to its label is a Microsoft extension}}
    S s = {}; // expected-note {{jump bypasses variable initialization}}
x:  return s.get();
  }
}
#endif

namespace test18 {
  struct A { ~A(); };
  struct B { const int &r; const A &a; };
  int f() {
    void *p = &&x;
    const A a = A();
  x:
    B b = { 0, a }; // ok
    goto *p;
  }
  int g() {
    void *p = &&x;
  x: // expected-note {{possible target of indirect goto}}
    B b = { 0, A() }; // expected-note {{jump exits scope of lifetime-extended temporary with non-trivial destructor}}
    goto *p; // expected-warning {{cannot jump}}
  }
}

#if __cplusplus >= 201103L
namespace std {
  typedef decltype(sizeof(int)) size_t;
  template<typename T> struct initializer_list {
    const T *begin;
    size_t size;
    initializer_list(const T *, size_t);
  };
}
namespace test19 {
  struct A { ~A(); };

  int f() {
    void *p = &&x;
    A a;
  x: // expected-note {{possible target of indirect goto}}
    std::initializer_list<A> il = { a }; // expected-note {{jump exits scope of lifetime-extended temporary with non-trivial destructor}}
    goto *p; // expected-warning {{cannot jump}}
  }
}

namespace test20 {
  struct A { ~A(); };
  struct B {
    const A &a;
  };

  int f() {
    void *p = &&x;
    A a;
  x:
    std::initializer_list<B> il = {
      a,
      a
    };
    goto *p;
  }
  int g() {
    void *p = &&x;
    A a;
  x: // expected-note {{possible target of indirect goto}}
    std::initializer_list<B> il = {
      a,
      { A() } // expected-note {{jump exits scope of lifetime-extended temporary with non-trivial destructor}}
    };
    goto *p; // expected-warning {{cannot jump}}
  }
}

namespace test21 {
  template<typename T> void f() {
  goto x; // expected-warning {{jump from this goto statement to its label is a Microsoft extension}}
    T t;  // expected-note {{jump bypasses variable with a non-trivial destructor}}
 x: return;
  }

  template void f<int>();
  struct X { ~X(); };
  template void f<X>(); // expected-note {{in instantiation of function template specialization}}
}
#else
namespace test21 {
  template<typename T> void f() {
  goto x; // expected-warning {{jump from this goto statement to its label is a Microsoft extension}}
    T t;  // expected-note {{jump bypasses variable with a non-trivial destructor}}
 x: return;
  }

  template void f<int>();
  struct X { ~X(); };
  template void f<X>(); // expected-note {{in instantiation of function template specialization}}
}
#endif

namespace test_recovery {
  // Test that jump scope checking recovers when there are unspecified errors
  // in the function declaration or body.

  void test(nexist, int c) { // expected-error {{unknown type name 'nexist'}}
    nexist_fn(); // expected-error {{}}
    goto nexist_label; // expected-error {{use of undeclared label}}
    goto a0; // expected-warning {{jump from this goto statement to its label is a Microsoft extension}}
    int a = 0; // expected-note {{jump bypasses variable initialization}}
    a0:;

    switch (c) {
    case $: // expected-error {{}}
    case 0:
      int x = 56; // expected-note {{jump bypasses variable initialization}}
    case 1: // expected-warning {{jump from switch statement to this case label bypasses variable initialization}}
      x = 10;
    }
  }
}

typedef int X;

void f()
{
  // ...
  goto lx;   // expected-warning {{jump from this goto statement to its label is a Microsoft extension}}
  // ...
ly:
  X a = 1;  // expected-note {{jump bypasses variable initialization}}
  // ...
lx:
  goto ly;   // ok, jump implies destructor call for 'a'
}

