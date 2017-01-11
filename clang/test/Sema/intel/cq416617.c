// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s
void foo() {
  _Atomic(int) i;
  const _Atomic(int) ci;
  __atomic_fetch_add(&i, 0, 0);
  __atomic_fetch_add_explicit(&i, 0, 0);
  __atomic_fetch_add_explicit_4(&i, 0, 0);

  __atomic_fetch_add(&ci, 0, 0); // expected-error {{address argument to atomic operation must be a pointer to non-const _Atomic type ('const _Atomic(int) *' invalid)}}
  __atomic_fetch_add_explicit(&ci, 0, 0);// expected-error {{address argument to atomic operation must be a pointer to non-const _Atomic type ('const _Atomic(int) *' invalid)}}
  __atomic_fetch_add_explicit_4(&ci, 0, 0);// expected-error {{address argument to atomic operation must be a pointer to non-const _Atomic type ('const _Atomic(int) *' invalid)}}

}
