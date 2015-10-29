// RUN: %clang_cc1 -Wformat-nonliteral -Werror=format=2 -fsyntax-only -Werror=format=2 -verify %s -DTEST1
// RUN: %clang_cc1 -Wformat-nonliteral -fsyntax-only -fintel-compatibility -verify %s -DTEST2
// RUN: %clang_cc1 -Werror=format=2 -fsyntax-only -fintel-compatibility -verify %s -DTEST3

extern void my_scanf(const char *format, ...)
    __attribute__((__format__(__scanf__, 1, 2)));

int foo(char *format) {
  int res;
#ifdef TEST1
  my_scanf(format, &res); // expected-error {{format string is not a string literal}}
#elif TEST2
  my_scanf(format, &res); // expected-warning {{format string is not a string literal}}
#elif TEST3
  my_scanf(format, &res); // expected-no-diagnostics
#else
#error Unknown test mode
#endif

  return res;
}

