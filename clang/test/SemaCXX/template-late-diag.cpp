// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility %s
// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility -DLATEDIAG %s
class foo1 { // expected-note {{candidate constructor (the implicit copy constructor) not viable}}
  int i;
};

class foo2 {
  char c;
  double d;
};

template <typename T>
foo1 bar(T *t, foo2 *f) {
  // The error below will be shown in the case with -DLATEDIAG only
  return f; // expected-error {{no viable conversion from returned value of type}}
}

// Without -DLATEDIAG there should not be any disagnostics because there is no template instantination
#ifdef LATEDIAG
void test() {
  int i = 25;
  foo2 f;
  foo1 foo = bar(&i, &f); // expected-note {{in instantiation of function template specialization}}
}
#endif
