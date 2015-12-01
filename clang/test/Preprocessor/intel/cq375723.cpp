// RUN: %clang_cc1 -fintel-compatibility -E -verify %s
// expected-no-errors

#line 12 incorrect filename // expected-warning{{expected a file name}}
#line 13 incorrect_filename.cpp // expected-warning{{expected a file name}}
#line 228 "correct_filename.cpp" // no warning
