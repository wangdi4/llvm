// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

//"vector-variants"="_ZGVbN4__Z3foov,_ZGVcN8__Z3foov,_ZGVdN8__Z3foov,_ZGVeN16__Z3foov,_ZGVbM4__Z3foov,_ZGVcM8__Z3foov,_ZGVdM8__Z3foov,_ZGVeM16__Z3foov"
//CHECK-NOT: _ZGVbN4__Z3foov
//CHECK-NOT: _ZGVcN8__Z3foov
//CHECK-NOT: _ZGVdN8__Z3foov
//CHECK-NOT: _ZGVeN16__Z3foov
//CHECK-NOT: _ZGVbM4__Z3foov
//CHECK-NOT: _ZGVcM8__Z3foov
//CHECK-NOT: _ZGVdM8__Z3foov
//CHECK-NOT: _ZGVeM16__Z3foov

struct Object {
public:
  int a;
  int b;
  int c;
};

#pragma omp declare simd
Object foo() { // expected-warning {{Vectorization of a functions with a struct or complex return type is not supported}}
  Object o;
  o.a = 1;
  o.b = 2;
  o.c = 3;
  return o;
}
