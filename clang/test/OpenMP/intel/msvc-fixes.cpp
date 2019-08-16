// RUN: %clang_cc1 -triple spir64 -aux-triple x86_64-pc-windows-msvc -fintel-ms-compatibility -fms-compatibility-version=19.15.26732.1 -fms-extensions -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device -fsyntax-only -fintel-compatibility -verify %s
// expected-no-diagnostics

void foo(__builtin_va_list bvl) {
  char * VaList = bvl;
  static_assert(sizeof(wchar_t) == 2, "sizeof wchar is 2 on Windows");
}

typedef char* va_list;
void __va_start(va_list* , ...);
