// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility -Wpedantic %s
extern "C" char *strstr(const char *s1, const char *s2);
void f() {
  char a[10]={};
  char b[10]={};
  // Warning only happens in -Wpedantic mode!
  if (strstr(a,b) > 0) {}//expected-warning {{ordered comparison between pointer and zero ('char *' and 'int') is an extension}}
}
