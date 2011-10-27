// RUN: %clang_cc1 %s -verify -pedantic -fsyntax-only

char f(char i, j) {} // expected-error {{Type specifier required for all declarations}}

kernel foo(void) {} // expected-error {{Type specifier required for all declarations}} expected-error {{kernel must return void in OpenCL}} expected-error {{kernel must return void in OpenCL}}
