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
  // expected-error@+3{{no matching function for call to 'Bar'}}
  // expected-note@8{{candidate template ignored: could not match 'const Ty *' against 'void (*)()}}
  // expected-note@10{{candidate template ignored: could not match 'const Ty *' against 'void (*)()}}
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

