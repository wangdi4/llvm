// RUN: %clang_cc1 -fintel-compatibility -verify %s

struct S {int a,b;};
void bar(S);

void foo() {
    bar({1,2}); // expected-warning{{generalized initializer lists are a C++11 extension}}
}
