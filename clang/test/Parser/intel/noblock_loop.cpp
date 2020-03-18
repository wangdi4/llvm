// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only %s -verify %s

// RUN: %clang_cc1 -fintel-compatibility-enable=PragmaBlockLoop \
// RUN: -fsyntax-only %s -verify %s

void error_check_noblock(int i, int *x, int *y)
{
  #pragma noblock_loop level(3:5)
  // expected-warning@-1{{extra tokens at end of '#pragma noblock_loop' - ignored}}
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }

  #pragma noblock_loop factor (16)
  // expected-warning@-1 {{extra tokens at end of '#pragma noblock_loop' - ignored}}
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }

  #pragma noblock_loop private(y)
  // expected-warning@-1 {{extra tokens at end of '#pragma noblock_loop' - ignored}}
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
}
