// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

extern "C" {
int bar()
{
  extern int iii;
  return iii;
}
}

extern "C++" {
int foo()
{
  extern int iii; // expected-warning{{declaration of 'iii' has a different language linkage}}
  return iii;
}
}
