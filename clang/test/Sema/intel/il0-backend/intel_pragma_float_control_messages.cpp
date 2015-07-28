// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s
//***INTEL: pragma float_control test

void www();

#pragma float_control ; // expected-warning {{missing '(' after '#pragma float_control' - ignoring}}
struct S {
  #pragma float_control (a, // expected-warning {{'push', 'pop', 'precise', 'source', 'double', 'extended' or 'except' is expected}}
  #pragma float_control (push, // expected-warning {{missing ')' after '#pragma float_control' - ignoring}}
  #pragma float_control (pop, // expected-warning {{missing ')' after '#pragma float_control' - ignoring}}
  #pragma float_control (precise, // expected-warning {{'on' or 'off' is expected}}
  #pragma float_control (source, // expected-warning {{'on' or 'off' is expected}}
  #pragma float_control (double, // expected-warning {{'on' or 'off' is expected}}
  #pragma float_control (extended, // expected-warning {{'on' or 'off' is expected}}
  #pragma float_control (except, // expected-warning {{'on' or 'off' is expected}}
  int a;
} d;

#pragma float_control (push)
#pragma float_control (pop)
#pragma float_control (precise, on // expected-warning {{missing ')' after '#pragma float_control' - ignoring}}
#pragma float_control (source, off // expected-warning {{missing ')' after '#pragma float_control' - ignoring}}
#pragma float_control (double, on, // expected-warning {{'push' is expected}}
#pragma float_control (extended, off, // expected-warning {{'push' is expected}}
#pragma float_control (except, on, push // expected-warning {{missing ')' after '#pragma float_control' - ignoring}}

#pragma float_control (except, on) // expected-error {{exception semantics cannot be enabled except in precise, source, double and extended modes}}
#pragma float_control (precise, on)
#pragma float_control (source, on)
#pragma float_control (double, on)
#pragma float_control (extended, on)
#pragma float_control (except, on)
#pragma float_control (precise, off) // expected-error {{exception semantics must be turned off before turning off this float_control mode}}
#pragma float_control (precise, off, push) // expected-error {{exception semantics must be turned off before turning off this float_control mode}}
#pragma float_control (source, off) // expected-error {{exception semantics must be turned off before turning off this float_control mode}}
#pragma float_control (source, off, push) // expected-error {{exception semantics must be turned off before turning off this float_control mode}}
#pragma float_control (double, off) // expected-error {{exception semantics must be turned off before turning off this float_control mode}}
#pragma float_control (double, off, push) // expected-error {{exception semantics must be turned off before turning off this float_control mode}}
#pragma float_control (extended, off) // expected-error {{exception semantics must be turned off before turning off this float_control mode}}
#pragma float_control (extended, off, push) // expected-error {{exception semantics must be turned off before turning off this float_control mode}}
#pragma float_control (except, off)
#pragma float_control (extended, off)
#pragma float_control (except, on) // expected-error {{exception semantics cannot be enabled except in precise, source, double and extended modes}}
