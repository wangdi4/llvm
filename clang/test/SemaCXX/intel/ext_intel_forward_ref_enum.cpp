// CQ#365886
// RUN: %clang_cc1 -fsyntax-only -verify %s -DTEST1
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DTEST2
// RUN: %clang_cc1 -fsyntax-only -fms-compatibility -verify %s -DTEST3
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -fms-compatibility -verify %s -DTEST3

#if TEST1

enum E1; // expected-error{{ISO C++ forbids forward references to 'enum' types}}

void foo() {
  enum E2; // expected-error{{ISO C++ forbids forward references to 'enum' types}}
}

class C1 {
public:
  C1() {
    n = TWO;
    bar = 1;
  }
  enum E3; // expected-error{{ISO C++ forbids forward references to 'enum' types}}
  int bar;
  enum E3 { ONE, TWO, THREE };
  E3 n;
};

#elif TEST2

enum E1; // expected-warning{{forward references to 'enum' types are an Intel extension}}

void foo() {
  enum E2; // expected-warning{{forward references to 'enum' types are an Intel extension}}
}

class C1 {
public:
  C1() {
    n = TWO;
    bar = 1;
  }
  enum E3; // expected-warning{{forward references to 'enum' types are an Intel extension}}
  int bar;
  enum E3 { ONE, TWO, THREE };
  E3 n;
};

#elif TEST3

enum E1; // expected-no-diagnostics

void foo() {
  enum E2; // expected-no-diagnostics
}

class C1 {
public:
  C1() {
    n = TWO;
    bar = 1;
  }
  enum E3; // expected-no-diagnostics
  int bar;
  enum E3 { ONE, TWO, THREE };
  E3 n;
};

#else

#error Unknown test mode

#endif
