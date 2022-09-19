// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility %s

void test1() {
Label: // expected-warning {{expected statement}}
}

void test2(int x) {
  switch(x) {
  case 0:
    // expected-warning@-1 {{label at end of switch compound statement: expected statement}}
  }
}

void test3(int x) {
  switch(x) {
  default:
    // expected-warning@-1 {{label at end of switch compound statement: expected statement}}
  }
}
