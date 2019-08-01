// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only %s -verify %s
// // RUN: %clang_cc1 -fintel-compatibility-enable=PragmaBlockLoop -fintel-compatibility-enable=PragmaBlockLoop -fsyntax-only %s -verify %s


void error_check(int i, int *x, int *y)
{
  // expected-warning@+1 {{'factor' cannot appear multiple times in '#pragma block_loop' - ignored}}
  #pragma block_loop level(3:5) factor (16) factor(12)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  // expected-warning@+1 {{'level' cannot appear multiple times in '#pragma block_loop' - ignored}}
  #pragma block_loop level(3:5) factor (16) level(5:8)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  // expected-warning@+1 {{'private' cannot appear multiple times in '#pragma block_loop' - ignored}}
  #pragma block_loop level(3:5) factor (16) private(x) private(y)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  // expected-warning@+1 {{invalid option 'levele'; expected factor, level or private - ignored}}
  #pragma block_loop levele(3:5)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
}
