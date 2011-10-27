// RUN: %clang_cc1 -fsyntax-only -verify %s

x; // expected-error{{Type specifier required for all declarations}}

f(int y) { return y; } // expected-error{{Type specifier required for all declarations}}
