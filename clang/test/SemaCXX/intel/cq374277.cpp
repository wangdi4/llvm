// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

namespace test1 __attribute__((visibility("hidden"))) { // expected-note{{surrounding namespace with visibility attribute starts here}}
#pragma GCC visibility pop // expected-warning{{#pragma visibility pop with no matching #pragma visibility push}}
}

#pragma GCC visibility pop // expected-warning{{#pragma visibility pop with no matching #pragma visibility push}}
