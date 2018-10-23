// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s
// RUN: %clang_cc1 -fintel-compatibility-enable=PragmaLoopCount  -fsyntax-only -verify %s
#include <stdint.h>

void simple_test() {
  int s = 0;
#pragma loop_count(1,2) min=1 max=10 avg=5
  for (int i = 0; i < 10; ++i){s = s + i;}

  // expected-warning@+1 {{'min' cannot appear multiple times in '#pragma loop_count' - ignored}}
  #pragma loop_count(1,2) min=1 max=10 avg=5 min = 1
  for (int i = 0; i < 10; ++i){s = s + i;}

  // expected-warning@+1 {{invalid loop count '#pragma loop_count' - ignored}}
  #pragma loop_count
  for (int i = 0; i < 10; ++i)

  // expected-warning@+1 {{invalid loop count '#pragma loop_count' - ignored}}
  #pragma loop_count xxx(1)
  for (int i = 0; i < 10; ++i)
  // expected-warning@+1 {{'avg' cannot appear multiple times in '#pragma loop_count' - ignored}}
  #pragma loop_count(1,2) min=1 max=10 avg=5 avg = 1
  for (int i = 0; i < 10; ++i){s = s + i;}

}

int main() {
  simple_test();
}
