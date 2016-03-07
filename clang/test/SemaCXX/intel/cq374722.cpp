// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -fintel-ms-compatibility -DMS -verify %s

class rv {
  // expected-note@+1 {{implicitly declared private here}}
  rv(rv const &);
};

struct thread {
  operator rv &();
};

char check(rv);

#ifndef MS
template <class T>
#endif
struct convertible {
  static thread from;
  // expected-warning@+1 {{calling a private constructor of class 'rv'}}
  static bool const value = sizeof(check(from));
};

