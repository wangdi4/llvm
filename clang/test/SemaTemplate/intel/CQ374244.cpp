// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility %s
template <class Ty>
inline void Foo(Ty *, const Ty *) {}
template <class Ty>
inline void Foo(Ty *, Ty *) {}

template <class Ty>
inline void Bar(Ty *, const Ty *) {}
template <class Ty>
inline void Bar(const Ty *, Ty *) {}

void Call() {
  void (*CP)();
  Foo(CP, CP);
  // expected-error@+3{{call to 'Bar' is ambiguous}}
  // expected-note@8{{candidate function}}
  // expected-note@10{{candidate function}}
  Bar(CP, CP);
}

// Original test case from CQ110092.
template <class _Ty>
inline void foo(_Ty **, _Ty **) {}
template <class _Ty>
inline void foo(_Ty **, const _Ty **) {}

typedef void (*pf)();

void foo() {
  pf *cp;
  foo(cp, cp);
}

