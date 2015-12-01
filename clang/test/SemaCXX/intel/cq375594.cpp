// RUN: %clang_cc1 -fintel-compatibility -verify %s
// REQUIRES: llvm-backend

// Check that we don't crash with assertion on IL0-specific attributes
// in non-IL0 configuration.

__declspec(avoid_false_share) int i; // expected-warning{{attribute 'avoid_false_share' is not supported}}

struct A {
  int i __attribute__((bnd_variable_size)); // expected-warning{{unknown attribute 'bnd_variable_size' ignored}}
} __attribute__((gcc_struct)); // expected-warning{{unknown attribute 'gcc_struct' ignored}}

int f(int x) __attribute__((bnd_legacy)); // expected-warning{{unknown attribute 'bnd_legacy' ignored}}
