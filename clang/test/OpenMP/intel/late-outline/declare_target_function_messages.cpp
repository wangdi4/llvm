// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 %s

//expected-error@+1 {{single function declaration is expected after 'declare target function' directive}}
#pragma ompx declare target function
int var;

//expected-error@+1 {{single function declaration is expected after 'declare target function' directive}}
[[ompx::directive(declare target function)]]
int varA;

//expected-error@+1 {{single function declaration is expected after 'declare target function' directive}}
#pragma ompx declare target function
int foo1(), foo2();

//expected-error@+1 {{single function declaration is expected after 'declare target function' directive}}
[[ompx::directive(declare target function)]]
int foo1A(), foo2A();

//expected-error@+1 {{use '#pragma ompx' for OpenMP extensions}}
#pragma omp declare target function
int foo3();

//expected-error@+1 {{use '#pragma ompx' for OpenMP extensions}}
[[omp::directive(declare target function)]]
int foo3A();

//expected-warning@+1 {{more than one 'device_type' clause is specified}}
#pragma ompx declare target function device_type(nohost) device_type(any)
int foo4();

//expected-warning@+1 {{more than one 'device_type' clause is specified}}
[[ompx::directive(declare target function device_type(nohost) device_type(any))]]
int foo4A();

//expected-error@+1 {{unexpected 'garbage' clause, only 'device_type' clauses expected}}
#pragma ompx declare target function device_type(host) garbage
int foo5();

//expected-error@+1 {{unexpected 'garbage' clause, only 'device_type' clauses expected}}
[[ompx::directive(declare target function device_type(host) garbage)]]
int foo5A();

namespace N {
  //expected-error@+1 {{single function declaration is expected after 'declare target function' directive}}
  #pragma ompx declare target function
}
namespace NA {
  //expected-error@+1 {{single function declaration is expected after 'declare target function' directive}}
  [[ompx::directive(declare target function)]]
}

//expected-warning@+1 {{unknown attribute 'sequence' ignored}}
[[ompx::sequence(ompx::directive(declare target function), omp::directive(declare simd))]]
int foo6();

//expected-error@+1 {{expected an OpenMP 'directive' or 'sequence' attribute argument}}
[[omp::sequence(directive(declare simd), ompx::sequence(ompx::directive(declare target function)))]]
int foo7();

// end INTEL_COLLAB
