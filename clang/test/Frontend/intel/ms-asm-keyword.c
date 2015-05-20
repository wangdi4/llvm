// CQ#369368
// RUN: %clang_cc1 -fasm-blocks -verify %s -DERROR
// RUN: %clang_cc1 -fintel-compatibility -fasm-blocks -verify %s -DOK

int main(void)
{
  long x;

#if OK

  _asm  // expected-no-diagnostics

#elif ERROR

  _asm  // expected-error {{use of undeclared identifier '_asm'}}

#else

#error Unknown test mode.

#endif
  {
    fistp x;
  }

  return 0;
}
