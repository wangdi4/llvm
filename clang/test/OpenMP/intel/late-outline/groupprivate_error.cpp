// INTEL_COLLAB
// RUN: %clang_cc1 -triple x86_64-apple-macos10.7.0  -verify -fopenmp-late-outline -fopenmp -ferror-limit 100  -o - %s
// RUN: %clang_cc1 -triple x86_64-apple-macos10.7.0 -fopenmp -fopenmp-late-outline -verify -fopenmp-simd  -ferror-limit 100  -o - %s

// RUN: %clang_cc1 -triple x86_64-apple-macos10.7.0 -DDEVICE -fopenmp-late-outline -fopenmp-is-device -verify -fopenmp -ferror-limit 100  -o - %s
// RUN: %clang_cc1 -triple x86_64-apple-macos10.7.0 -DDEVICE -fopenmp -fopenmp-late-outline -fopenmp-is-device -verify -fopenmp-simd  -ferror-limit 100  -o - %s
#ifndef DEVICE
namespace {
  int ooo;
  // expected-note@+2 {{'#pragma omp groupprivate' is specified here}}
  // expected-note@+1 {{'#pragma omp groupprivate' is specified here}}
  #pragma omp groupprivate (ooo) device_type(nohost)
};

int xoo() {
// expected-warning@+1 {{groupprivate directive for variable 'ooo' is ignored for x86-64/host compilation}}
  ::ooo++;
// expected-warning@+1 {{groupprivate directive for variable 'ooo' is ignored for x86-64/host compilation}}
  return ::ooo;
}
#pragma omp begin declare target device_type(host)
int x;
#pragma omp groupprivate (x) device_type(nohost)
void foo() {
// expected-error@+1 {{'device_type(host)' does not match previously specified 'device_type(nohost)' for the same declaration}}
  x++;
}
#pragma omp end declare target
#pragma omp begin declare variant match(device={kind(host)})
void bar() {
  static int y;
  // expected-note@+1 2 {{'#pragma omp groupprivate' is specified here}}
  #pragma omp groupprivate(y) device_type(nohost) // expected-note {{defined as groupprivate}}
  // expected-warning@+1 {{groupprivate directive for variable 'y' is ignored for x86-64/host compilation}}
  #pragma omp target map(y) // expected-error {{groupprivate variables are not allowed in 'map' clause}}
  y = 0; // expected-warning {{groupprivate directive for variable 'y' is ignored for x86-64/host compilation}}
}
#pragma omp end declare variant

void yoo() {
  #pragma omp parallel
  {
    static int local_sum;
    // expected-note@+1 2{{'#pragma omp groupprivate' is specified here}}
    #pragma omp groupprivate(local_sum)
    // expected-warning@+1 2{{groupprivate directive for variable 'local_sum' is ignored for x86-64/host compilation}}
    #pragma omp parallel reduction( +:local_sum ) // expected-no-error
    for ( int i = 0; i < 10; i++ );
  }
}

#else
namespace {
  int ooo;
  // expected-note@+1 2 {{'#pragma omp groupprivate' is specified here}}
  #pragma omp groupprivate (ooo) device_type(nohost)
};

int xoo() {
// expected-warning@+1 {{groupprivate directive for variable 'ooo' is ignored for x86-64/host compilation}}
  ::ooo++;
// expected-warning@+1 {{groupprivate directive for variable 'ooo' is ignored for x86-64/host compilation}}
  return ::ooo;
}
#pragma omp begin declare target device_type(host)
int x;
#pragma omp groupprivate (x) device_type(nohost)
void foo() {
// expected-error@+1 {{'device_type(host)' does not match previously specified 'device_type(nohost)' for the same declaration}}
  x++;
}
#pragma omp end declare target
#pragma omp begin declare variant match(device={kind(host)})
void bar() {
  static int y;
  #pragma omp groupprivate(x) device_type(nohost)
  #pragma omp target map(y)
  y = 0;
}
#pragma omp end declare variant
#endif
// end INTEL_COLLAB
