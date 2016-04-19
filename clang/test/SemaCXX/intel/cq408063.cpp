// RUN: %clang_cc1 -triple x86_64-windows -fsyntax-only -verify -fintel-compatibility -fintel-ms-compatibility %s

struct A {
  wchar_t a1[8];
  char    a2[8];
} a = {
  (const wchar_t *)L"foo",
  (const char *)"bar"
};

struct B {
  wchar_t b1[8];
  char    b2[8];
} b = {
  static_cast<const wchar_t *>(L"foo"),
  static_cast<const char *>("bar")
};

const char c1[8] = (const char *)"a2";
// expected-error@-1 {{array initializer must be an initializer list or string literal}}
const wchar_t c2[8] = (const wchar_t *)L"a2";
// expected-error@-1 {{array initializer must be an initializer list}}

void f1(char * t = "");
// expected-warning@-1 {{conversion from string literal to 'char *' is deprecated}}
void f2(wchar_t * t = L"");
// expected-warning@-1 {{conversion from string literal to 'wchar_t *'}}

char *d1 = true ? "a" : "b";
// expected-warning@-1 {{conversion from string literal to 'char *' is deprecated}}
char *d2 = true ? d1 : "c";
// expected-warning@-1 {{conversion from string literal to 'char *' is deprecated}}
char *d3 = true ? "d" : d2;
// expected-warning@-1 {{conversion from string literal to 'char *' is deprecated}}
const char *d4 = 0;
const char *d5 = 0;
char *d6 = true ? d4 : d5;
// expected-error@-1 {{cannot initialize a variable of type 'char *' with an lvalue of type 'const char *'}}

wchar_t *e1 = true ? L"a" : L"b";
// expected-warning@-1 {{conversion from string literal to 'wchar_t *'}}
wchar_t *e2 = true ? e1 : L"c";
// expected-warning@-1 {{conversion from string literal to 'wchar_t *'}}
wchar_t *e3 = true ? L"d" : e2;
// expected-warning@-1 {{conversion from string literal to 'wchar_t *'}}
const wchar_t *e4 = 0;
const wchar_t *e5 = 0;
wchar_t *e6 = true ? e4 : e5;
// expected-error@-1 {{cannot initialize a variable of type 'wchar_t *'}}
