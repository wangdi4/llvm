// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s
// RUN: %clang_cc1 -fintel-compatibility-enable=PragmaLoopCount  -fsyntax-only -verify %s
#include <stdint.h>

void simple_test() {
  int s = 0;
#pragma loop_count(1,2) min=1 max=10 avg=5
  for (int i = 0; i < 10; ++i){s = s + i;}

  // expected-warning@+1 {{invalid loop count '#pragma loop_count' - ignored}}
  #pragma loop_count
  for (int i = 0; i < 10; ++i)

  // expected-warning@+1 {{invalid loop count '#pragma loop_count' - ignored}}
  #pragma loop_count xxx(1)
  for (int i = 0; i < 10; ++i)

  #pragma loop_count(1,2), min=1, max=10, avg=5
  for (int i = 0; i < 10; ++i){s = s + i;}

  #pragma loop_count min(1), max=10, avg(5)
  for (int i = 0; i < 10; ++i){s = s + i;}

  // expected-warning@+1 {{invalid loop count '#pragma loop_count' - ignored}}
  #pragma loop_count min(1) #pragma
  for (int i = 0; i < 10; ++i){s = s + i;}
}

template <int min, int max>
void foo(int j) {
  int i, a[20], b[20];
  #pragma loop_count(1+(max+min)+1, 21, min) min=(max+min)+1,  max((max+min)+1), avg=20
  for (i = 0; i < 10; ++i) {
    a[i] = b[i] = 0;
  }
}

int main() {
  simple_test();
  foo<10, 20>(5);
}
