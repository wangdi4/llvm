// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

int& inline foo(); // expected-warning {{"inline" is not allowed}}
