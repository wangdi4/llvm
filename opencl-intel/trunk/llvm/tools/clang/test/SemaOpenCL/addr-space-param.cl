// RUN: %clang_cc1 %s -verify -pedantic -fsyntax-only

void foo(const __attribute__((address_space(0))) int i) {
}
