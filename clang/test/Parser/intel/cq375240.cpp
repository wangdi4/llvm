// RUN: %clang_cc1 -fintel-compatibility -verify %s

union U
{
  struct _s
  {
    int a, b, c;
  } __data;
  char __size[40];
} pthread_mutex_t;

struct S {
  U  u;
  void foo();
};

void S::foo()
{
  u = { { 1, 2, 3 } }; // expected-warning{{generalized initializer lists are a C++11 extension}}
}
