// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s

#pragma region wwww // expected-warning {{#pragma region unclosed at end of file}}

