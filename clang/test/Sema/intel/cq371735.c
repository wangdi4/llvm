// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s

int foo() { return 0; }

// expected-no-diagnostics
void check() {
  long y;
  __asm__("nop" : : "m"(foo()));
  __asm__("mov %1, %0"
          : "=r"(y)
          : "m"((long)55));
  __asm__("mov %1, %0"
          : "=r"(y)
          : "m"(y++));
  __asm__(""
          :
          : "m"(({
            unsigned __v;
            __v;
          })));
}

