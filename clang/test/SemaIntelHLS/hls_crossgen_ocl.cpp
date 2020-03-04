// RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic -triple spir64-unknown-unknown-intelfpga -aux-triple x86_64-pc-windows-msvc %s
// expected-no-diagnostics

void foo(__builtin_va_list bvl) {
  char * VaList = bvl;
  static_assert(sizeof(wchar_t) == 4, "sizeof wchar is 4 on SPIR64 target");
  static_assert(sizeof(long) == 4, "sizeof long is 4 on Windows");
  static_assert(alignof(long) == 4, "alignof long is 4 on Windows");
}
