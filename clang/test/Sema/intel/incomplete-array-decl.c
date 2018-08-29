// RUN: %clang_cc1 -fintel-compatibility -verify -emit-llvm -o - %s

struct foo a[];   // expected-warning {{tentative array definition assumed to have one element}} \
                  // expected-error {{tentative definition has type 'struct foo [1]' that is never completed}}
                  // expected-note@-2 {{forward declaration of 'struct foo'}}
struct foo b[10]; // expected-error {{tentative definition has type}}
                  // expected-note@-4 {{forward declaration of 'struct foo'}}
