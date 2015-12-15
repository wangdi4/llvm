// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu  -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple i386-unknown-linux-gnu  -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-win32  -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple i386-pc-win32  -fsyntax-only -verify %s

struct a {
  int b;
};

struct a test __attribute__((interrupt)); // expected-error {{'interrupt' attribute only applies to functions}}

__attribute__((interrupt)) int foo1(void) { return 0; }             // expected-error {{interrupt service routine must have void return value}}
__attribute__((interrupt)) void foo2(void) {}                       // expected-error {{interrupt service routine can only have a pointer argument and an optional integer argument}}
__attribute__((interrupt)) void foo3(void *a, unsigned b, int c) {} // expected-error {{interrupt service routine can only have a pointer argument and an optional integer argument}}
__attribute__((interrupt)) void foo4(int a) {}                      // expected-error {{interrupt service routine should have a pointer as the first argument}}
__attribute__((interrupt)) void foo5(void *a, float b) {}           // expected-error {{interrupt service routine should have one of unsigned integer types as the second argument}}
__attribute__((interrupt)) void foo6(float *a, int b) {}            // expected-error {{interrupt service routine should have one of unsigned integer types as the second argument}}
__attribute__((interrupt)) void foo7(int *a, unsigned b) {}
__attribute__((interrupt)) void foo8(int *a) {}
int main(int argc, char **argv) {
  void *ptr = (void *)&foo7;
  (void)ptr;
  foo7((int *)argv, argc); // expected-error {{interrupt service routine can't be called directly}}
  foo8((int *)argv);       // expected-error {{interrupt service routine can't be called directly}}
  return 0;
}

