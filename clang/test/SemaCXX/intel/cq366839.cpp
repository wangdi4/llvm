// CQ#366839
// RUN: %clang_cc1 -fsyntax-only -std=c++0x -Wc++11-narrowing -verify %s -DTEST1
// RUN: %clang_cc1 -fsyntax-only -std=c++0x -Wc++11-narrowing -fintel-compatibility -verify %s -DTEST2
// RUN: %clang_cc1 -fsyntax-only -std=c++0x -Wc++11-narrowing -fms-compatibility -verify %s -DTEST2
// RUN: %clang_cc1 -fsyntax-only -std=c++0x -Wc++11-narrowing -fintel-compatibility -fms-compatibility -verify %s -DTEST2

#if TEST1

const unsigned char a = {-1}; // expected-error{{constant expression evaluates to -1 which cannot be narrowed to type 'unsigned char'}} \
// expected-note {{insert an explicit cast to silence this issue}}
unsigned int b = {-1.0}; // expected-error{{type 'double' cannot be narrowed to 'unsigned int' in initializer list}} \
// expected-note {{insert an explicit cast to silence this issue}}
char c = {b}; // expected-error{{non-constant-expression cannot be narrowed from type 'unsigned int' to 'char' in initializer list}} \
// expected-note {{insert an explicit cast to silence this issue}}

#elif TEST2

const unsigned char a = {-1}; // expected-warning{{constant expression evaluates to -1 which cannot be narrowed to type 'unsigned char' in C++11}} \
// expected-note {{insert an explicit cast to silence this issue}}
unsigned int b = {-1.0}; // expected-warning{{type 'double' cannot be narrowed to 'unsigned int' in initializer list in C++11}} \
// expected-note {{insert an explicit cast to silence this issue}}
char c = {b}; // expected-warning{{non-constant-expression cannot be narrowed from type 'unsigned int' to 'char' in initializer list}} \
// expected-note {{insert an explicit cast to silence this issue}}

#else

#error Unknown test mode

#endif
