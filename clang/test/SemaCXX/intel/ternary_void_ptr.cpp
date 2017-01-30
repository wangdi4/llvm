// RUN: %clang_cc1 -fintel-compatibility -gnu-permissive -fsyntax-only -verify %s
// CQ417359
// expected-no-diagnostics
struct Struct {};

extern void* void_result();

void* func(int a){
  Struct* s = 0;
  return ( a == 5) ? s : void_result();
}
