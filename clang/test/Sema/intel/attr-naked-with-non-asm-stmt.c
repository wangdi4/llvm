// CQ#371340
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

// Correct cases - no diagnostics.
int __attribute__((naked)) correct1() {
  asm("movl $42, %eax");
  asm("ret");
}

__declspec(naked) int correct2() {
  asm("movl $42, %eax");
  asm("ret");
}

// Incorrect cases - warning should be emitted.
int __attribute__((naked)) incorrect1() {
// expected-warning@-1{{attribute 'naked' ignored on function with non-ASM statements}}
  int a = 5;
  return 42;
}

__declspec(naked) int incorrect2() {
// expected-warning@-1{{attribute 'naked' ignored on function with non-ASM statements}}
  int a = 5;
  return 42;
}
