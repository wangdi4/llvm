// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility -gnu-permissive -triple x86_64-unknown-linux-gnu %s

void* f1(void* p) {
  return p - sizeof(int);
  // expected-warning@-1{{arithmetic on a pointer to void}}
}

unsigned long f2(void* p1, void* p2) {
  return p1 - p2;
  // expected-warning@-1{{arithmetic on pointers to void}}
}
