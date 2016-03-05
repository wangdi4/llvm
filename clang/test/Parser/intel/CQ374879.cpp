// RUN: %clang_cc1 -fintel-compatibility -verify %s

int main ()
{
  int n{1};                  // expected-warning{{generalized initializer lists are a C++11 extension}}
  int m[2]{2, 3};            // expected-warning{{generalized initializer lists are a C++11 extension}}
  struct {int a, b;} s{4,5}; // expected-warning{{generalized initializer lists are a C++11 extension}}
  int *p = new int[2]{6,7};  // expected-warning{{generalized initializer lists are a C++11 extension}}
  return (n != 1) | ((m[0] != 2) << 1) | ((m[1] != 3) << 2) | ((s.a != 4) << 3) |
         ((s.b != 5) << 4) | ((p[0] != 6) << 5) | ((p[1] != 7) << 6);
}
