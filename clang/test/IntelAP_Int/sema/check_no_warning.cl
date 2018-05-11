// Checks that there are no warnings issued. In particular, there should be no warnings about mismatched sizes

// RUN: %clang -cc1 -O3 -disable-llvm-passes %s -fsyntax-only -verify
// expected-no-diagnostics

#include "../ihc_apint.h"

kernel void foo() {
  int17_tt x17_s = 5;
  int17_tt y17_s = x17_s + 17;

  uint17_tt x17_u = 5;
  uint17_tt y17_u = x17_u + 17;
}
