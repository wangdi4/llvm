// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -fintel-ms-compatibility -Wno-c++11-compat-deprecated-writable-strings -verify %s
// expected-no-diagnostics
void foo(void *pv) {}
int main() {
  foo("123");
  foo(L"123");
  return 0;
}

