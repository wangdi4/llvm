// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s 
//***INTEL: pragma conform test
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
#pragma conform // expected-warning {{missing '(' after '#pragma conform' - ignoring}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}

int main() {
  
  
  for (int i = 0; i < 10; ++i);
  ++i;          // expected-error {{use of undeclared identifier 'i'}}
#pragma conform forScope  // expected-warning {{missing '(' after '#pragma conform' - ignoring}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i = 0; i < 10; ++i);
  ++i;          // expected-error {{use of undeclared identifier 'i'}}
#pragma conform ( // expected-warning {{'forScope' is expected}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i = 0; i < 10; ++i);
  ++i;          // expected-error {{use of undeclared identifier 'i'}}
#pragma conform (qqq // expected-warning {{'forScope' is expected}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
#pragma conform (9 // expected-warning {{'forScope' is expected}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i = 0; i < 10; ++i);
  ++i;          // expected-error {{use of undeclared identifier 'i'}}
#pragma conform (forScope // expected-warning {{',' is expected}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
#pragma conform (forScope 7 // expected-warning {{',' is expected}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
#pragma conform (forScope show // expected-warning {{',' is expected}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i = 0; i < 10; ++i);
  ++i;          // expected-error {{use of undeclared identifier 'i'}}
#pragma conform (forScope,  // expected-warning {{'show', 'push', 'pop', 'on' or 'off' is expected}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i = 0; i < 10; ++i);
  ++i;          // expected-error {{use of undeclared identifier 'i'}}
#pragma conform (forScope, 1 // expected-warning {{'show', 'push', 'pop', 'on' or 'off' is expected}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i = 0; i < 10; ++i);
  ++i;          // expected-error {{use of undeclared identifier 'i'}}
#pragma conform (forScope, on // expected-warning {{missing ')' after '#pragma conform' - ignoring}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i = 0; i < 10; ++i);
  ++i;          // expected-error {{use of undeclared identifier 'i'}}
#pragma conform (forScope, off // expected-warning {{missing ')' after '#pragma conform' - ignoring}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i = 0; i < 10; ++i);
  ++i;          // expected-error {{use of undeclared identifier 'i'}}
#pragma conform (forScope, push // expected-warning {{',' or ')' is expected}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i = 0; i < 10; ++i);
  ++i;          // expected-error {{use of undeclared identifier 'i'}}
#pragma conform (forScope, pop // expected-warning {{',' or ')' is expected}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i = 0; i < 10; ++i);
  ++i;          // expected-error {{use of undeclared identifier 'i'}}
#pragma conform (forScope, show // expected-warning {{missing ')' after '#pragma conform' - ignoring}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i = 0; i < 10; ++i);
  ++i;          // expected-error {{use of undeclared identifier 'i'}}
#pragma conform (forScope, off) 
#pragma conform (forScope, show) // expected-warning {{forScope behavior is non-standard}}
  for (int i = 0; i < 10; ++i);
  ++i;        
#pragma conform (forScope, on)
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i1 = 0; i1 < 10; ++i1);
  ++i1;          // expected-error {{use of undeclared identifier 'i1'}}
#pragma conform (forScope, off,  // expected-warning {{missing ')' after '#pragma conform' - ignoring}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i1 = 0; i1 < 10; ++i1);
  ++i1;          // expected-error {{use of undeclared identifier 'i1'}}
#pragma conform (forScope, on "" // expected-warning {{missing ')' after '#pragma conform' - ignoring}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i1 = 0; i1 < 10; ++i1);
  ++i1;          // expected-error {{use of undeclared identifier 'i1'}}
#pragma conform (forScope, push, on // expected-warning {{missing ')' after '#pragma conform' - ignoring}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i1 = 0; i1 < 10; ++i1);
  ++i1;          // expected-error {{use of undeclared identifier 'i1'}}
#pragma conform (forScope, pop, off // expected-warning {{missing ')' after '#pragma conform' - ignoring}}
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
  for (int i1 = 0; i1 < 10; ++i1);
  ++i1;          // expected-error {{use of undeclared identifier 'i1'}}
#pragma conform (forScope, push, off)
#pragma conform (forScope, show) // expected-warning {{forScope behavior is non-standard}}
  for (int i1 = 0; i1 < 10; ++i1);
  ++i1;          
#pragma conform (forScope, pop)
#pragma conform (forScope, show) // expected-warning {{forScope behavior is non-standard}}
  for (int i2 = 0; i2 < 10; ++i2);
  ++i2;

#pragma conform (forScope, push, off)
#pragma conform (forScope, show) // expected-warning {{forScope behavior is non-standard}}
#pragma conform (forScope, push, on)
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
#pragma conform (forScope, push, id1, off)
#pragma conform (forScope, show) // expected-warning {{forScope behavior is non-standard}}
#pragma conform (forScope, push, id2, off)
#pragma conform (forScope, show) // expected-warning {{forScope behavior is non-standard}}
#pragma conform (forScope, pop, id1)
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
#pragma conform (forScope, pop, id3)
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
#pragma conform (forScope, pop)
#pragma conform (forScope, show) // expected-warning {{forScope behavior is standard}}
#pragma conform (forScope, pop)
#pragma conform (forScope, show) // expected-warning {{forScope behavior is non-standard}}
#pragma conform (forScope, pop)
#pragma conform (forScope, show) // expected-warning {{forScope behavior is non-standard}}
#pragma conform (forScope, pop)
#pragma conform (forScope, show) // expected-warning {{forScope behavior is non-standard}}

  
  return 0;
  
}
