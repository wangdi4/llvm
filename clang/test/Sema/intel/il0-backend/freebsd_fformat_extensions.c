// RUN: %clang_cc1 -fsyntax-only -verify -fformat-extensions %s
// Test for the FreeBSD kernel format extensions support in iclang.

int printf(const char *restrict, ...);

void test(int q, char *b) {
  const char kFormat4[] = "%y"; // OK, no warnings
  printf(kFormat4, 5); // OK
  printf("%y", 5); // OK
  printf("0x%b\n", q, b); // OK
  printf("%b\n", q); // expected-warning {{more '%' conversions than data arguments}}
  printf("%b\n", q, q); // expected-warning {{format specifies type 'char *' but the argument has type 'int'}}
  printf("%b\n", b, b); // expected-warning {{format specifies type 'int' but the argument has type 'char *'}}
  printf("%b\n", 4321, "a"); // OK
}

