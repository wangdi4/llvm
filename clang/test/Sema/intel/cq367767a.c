// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DOK

void foo()
{
  return 1; // expected-warning{{void function 'foo' should not return a value}}
}
