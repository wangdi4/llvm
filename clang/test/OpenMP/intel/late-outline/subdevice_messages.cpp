// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fsyntax-only -fopenmp -fintel-compatibility \
// RUN: -fopenmp-late-outline %s
//
// Check invalid arguments to subdevice clause

void func() {
  int level, start, len, str;
  unsigned int ulevel;
  #pragma omp target subdevice // expected-error {{expected '(' after 'subdevice'}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target subdevice() // expected-error {{expected expression}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target subdevice(start,len,str) // expected-warning {{expected nonnegative constant integer for level in 'subdevice' clause - ignored}}
                                              // expected-warning@-1 {{missing ':' after argument - ignoring}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target subdevice(start:len,str) // expected-warning {{missing ':' after len - ignoring}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target subdevice(start len:str) // expected-warning {{missing ':' after len - ignoring}}
                                              // expected-error@-1 {{expected ')'}}
                                              // expected-note@-2 {{to match this '('}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target subdevice(start,,) // expected-warning {{expected nonnegative constant integer for level in 'subdevice' clause - ignored}}
                                        // expected-error@-1 {{expected expression}}
                                        // expected-warning@-2 {{missing ':' after argument - ignoring}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target subdevice(start:,) // expected-error {{expected expression}}
                                      // expected-warning@-1 {{missing ':' after argument - ignoring}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target subdevice(-1) // expected-error {{argument to 'subdevice' clause must be a non-negative integer value}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target subdevice(-1,start) // expected-warning {{missing ':' after argument - ignoring}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target subdevice(1:0) // expected-error {{argument to 'subdevice' clause must be a strictly positive integer value}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target subdevice(1:1:0) // expected-error {{argument to 'subdevice' clause must be a strictly positive integer value}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target subdevice(1:1:1:1) // expected-error {{expected ')'}}
                                        // expected-note@-1 {{to match this '('}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target subdevice(start:0) // expected-error {{'subdevice' clause must be a strictly positive integer value}}
  for (int i = 0; i < 16; ++i) {
  }
  float x;
  #pragma omp target subdevice(x) // expected-error {{expression must have integral or unscoped enumeration type, not 'float'}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp parallel for subdevice(start:len) // expected-error {{unexpected OpenMP clause 'subdevice' in directive '#pragma omp parallel for'}}
  for(int i=0; i<10; i++) {
  }
  #pragma omp target subdevice(ulevel,start:len:str) // expected-warning {{expected nonnegative constant integer for level in 'subdevice' clause - ignored}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target subdevice(2,start:len:str) // expected-warning {{'subdevice' level value must be 0 or 1 - ignored, assuming 0}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target subdevice(2147483647,start:len:str) // expected-warning {{'subdevice' level value must be 0 or 1 - ignored, assuming 0}}
  for (int i = 0; i < 16; ++i) {
  }

  // must have only one subdevice clause
  //
  #pragma omp target subdevice(start) subdevice(len) // expected-error {{directive '#pragma omp target' cannot contain more than one 'subdevice' clause}}
  {}
  #pragma omp target data use_device_addr(str) subdevice(start) subdevice(len)
  // expected-error@-1 {{directive '#pragma omp target data' cannot contain more than one 'subdevice' clause}}
  {}
  #pragma omp target enter data device(str) subdevice(len) map(alloc: start) \
                subdevice(1)
  // expected-error@-1 {{directive '#pragma omp target enter data' cannot contain more than one 'subdevice' clause}}
  #pragma omp target exit data device(str) subdevice(len) map(release: start) \
                subdevice(1)
  // expected-error@-1 {{directive '#pragma omp target exit data' cannot contain more than one 'subdevice' clause}}
  #pragma omp target parallel subdevice(start) subdevice(len:str)
  // expected-error@-1 {{directive '#pragma omp target parallel' cannot contain more than one 'subdevice' clause}}
  {}
  #pragma omp target teams distribute simd subdevice(start:len) subdevice(1)
  // expected-error@-1 {{directive '#pragma omp target teams distribute simd' cannot contain more than one 'subdevice' clause}}
  for(int i=0; i < 10; ++i) {
  }
  #pragma omp target teams subdevice(start:1) subdevice(1:len)
  // expected-error@-1 {{directive '#pragma omp target teams' cannot contain more than one 'subdevice' clause}}
  for(int i=0; i < 10; ++i) {
  }
  #pragma omp target simd subdevice(1) subdevice(start)
  // expected-error@-1 {{directive '#pragma omp target simd' cannot contain more than one 'subdevice' clause}}
  for(int i=0; i < 10; ++i) {
  }
}
// end INTEL_COLLAB
