// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility %s

template <class Ty>
inline void Foo(Ty *, const Ty *) { Ty::error; }
// expected-error@+2 {{type 'void ()' cannot be used prior to '::' because it has no members}}
template <class Ty>
inline void Foo(Ty *, Ty *) { Ty::error; }

void Call() {
  void (*CP)();
// expected-note@+1 {{in instantiation of function template specialization 'Foo<void ()>' requested here}}
  Foo(CP, CP);
}
