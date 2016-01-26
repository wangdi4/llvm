// RUN: %clang_cc1 -fintel-compatibility -verify %s

typedef void(FFF)(int i=17); // expected-warning{{default arguments can only be specified for parameter}}
FFF *F;

void foo(bool b) {
  F(26);
}

