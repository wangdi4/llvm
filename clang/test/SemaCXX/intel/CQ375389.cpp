// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -fintel-ms-compatibility -Wno-c++11-compat-deprecated-writable-strings -verify %s
// expected-no-diagnostics
typedef wchar_t * LPWST;
LPWST ps1 = (1 == 1) ? L"A" : L"BC";
LPWST ps2 = (1 == 1) ? L"A" : L"C";

