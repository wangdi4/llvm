// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s
// expected-no-diagnostics
__declspec(align(64)) float rn[2];
