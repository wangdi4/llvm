// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fsyntax-only -fopenmp -fintel-compatibility \
// RUN: -fopenmp-late-outline %s
//
// Check invalid arguments to ompx_places clause

void func() {
  int level, start, len, str;
  unsigned int ulevel;
  #pragma omp target ompx_places // expected-error {{expected '(' after 'ompx_places'}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target ompx_places() // expected-error {{expected expression}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target ompx_places(start,len,str) // expected-error {{unknown domain modifier in 'ompx_places' clause; must be one of 'numa_domain' or 'subnuma_domain'}}
                                               // expected-warning@-1 {{missing ':' after argument - ignoring}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target ompx_places(start:len,str) // expected-warning {{missing ':' after len - ignoring}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target ompx_places(start len:str) // expected-warning {{missing ':' after len - ignoring}}
                                              // expected-error@-1 {{expected ')'}}
                                              // expected-note@-2 {{to match this '('}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target ompx_places(numa,,) // expected-error {{unknown domain modifier in 'ompx_places' clause; must be one of 'numa_domain' or 'subnuma_domain'}}
                                         // expected-error@-1 {{expected expression}}
                                         // expected-warning@-2 {{missing ':' after argument - ignoring}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target ompx_places(start:,) // expected-error {{expected expression}}
                                      // expected-warning@-1 {{missing ':' after argument - ignoring}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target ompx_places(-1) // expected-error {{argument to 'ompx_places' clause must be a non-negative integer value}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target ompx_places(-1,start) // expected-warning {{missing ':' after argument - ignoring}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target ompx_places(1:0) // expected-error {{argument to 'ompx_places' clause must be a strictly positive integer value}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target ompx_places(1:1:0) // expected-error {{argument to 'ompx_places' clause must be a strictly positive integer value}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target ompx_places(1:1:1:1) // expected-error {{expected ')'}}
                                        // expected-note@-1 {{to match this '('}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp target ompx_places(start:0) // expected-error {{'ompx_places' clause must be a strictly positive integer value}}
  for (int i = 0; i < 16; ++i) {
  }
  float x;
  #pragma omp target ompx_places(x) // expected-error {{expression must have integral or unscoped enumeration type, not 'float'}}
  for (int i = 0; i < 16; ++i) {
  }
  #pragma omp parallel for ompx_places(start:len) // expected-error {{unexpected OpenMP clause 'ompx_places' in directive '#pragma omp parallel for'}}
  for(int i=0; i<10; i++) {
  }
  #pragma omp target ompx_places(subnuma,start:len:str) // expected-error {{unknown domain modifier in 'ompx_places' clause; must be one of 'numa_domain' or 'subnuma_domain'}}
  for (int i = 0; i < 16; ++i) {
  }

  // must have only one ompx_places clause
  //
  #pragma omp target ompx_places(start) ompx_places(len) // expected-error {{directive '#pragma omp target' cannot contain more than one 'ompx_places' clause}}
  {}
  #pragma omp target data use_device_addr(str) ompx_places(start) ompx_places(len)
  // expected-error@-1 {{directive '#pragma omp target data' cannot contain more than one 'ompx_places' clause}}
  {}
  #pragma omp target enter data device(str) ompx_places(len) map(alloc: start) \
                ompx_places(1)
  // expected-error@-1 {{directive '#pragma omp target enter data' cannot contain more than one 'ompx_places' clause}}
  #pragma omp target exit data device(str) ompx_places(len) map(release: start) \
                ompx_places(1)
  // expected-error@-1 {{directive '#pragma omp target exit data' cannot contain more than one 'ompx_places' clause}}
  #pragma omp target parallel ompx_places(start) ompx_places(len:str)
  // expected-error@-1 {{directive '#pragma omp target parallel' cannot contain more than one 'ompx_places' clause}}
  {}
  #pragma omp target teams distribute simd ompx_places(start:len) ompx_places(1)
  // expected-error@-1 {{directive '#pragma omp target teams distribute simd' cannot contain more than one 'ompx_places' clause}}
  for(int i=0; i < 10; ++i) {
  }
  #pragma omp target teams ompx_places(start:1) ompx_places(1:len)
  // expected-error@-1 {{directive '#pragma omp target teams' cannot contain more than one 'ompx_places' clause}}
  for(int i=0; i < 10; ++i) {
  }
  #pragma omp target simd ompx_places(1) ompx_places(start)
  // expected-error@-1 {{directive '#pragma omp target simd' cannot contain more than one 'ompx_places' clause}}
  for(int i=0; i < 10; ++i) {
  }
}
// end INTEL_COLLAB
