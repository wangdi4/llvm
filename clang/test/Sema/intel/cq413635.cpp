// CQ#413635
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify -std=c++98 %s
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify -std=c++11 %s
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify -std=c++14 %s
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify -std=gnu++98 %s
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify -std=gnu++11 %s
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify -std=gnu++14 %s

// expected-no-diagnostics
struct myInt {
  myInt() { v = 0;}
  myInt(int x) { v = x;}
  int v;
};
myInt arr1[100];
void fn()
{
  arr1[:] = 5;
}
