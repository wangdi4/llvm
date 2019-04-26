// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s
// expected-no-diagnostics
__declspec(align(64,60)) float r[1];
__declspec(align(64)) float rn[2];
