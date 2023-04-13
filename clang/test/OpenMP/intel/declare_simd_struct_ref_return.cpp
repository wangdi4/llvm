// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

//CHECK-DAG: _ZGVbN2__Z3foov
//CHECK-DAG: _ZGVcN4__Z3foov
//CHECK-DAG: _ZGVdN4__Z3foov
//CHECK-DAG: _ZGVeN8__Z3foov
//CHECK-DAG: _ZGVbM2__Z3foov
//CHECK-DAG: _ZGVcM4__Z3foov
//CHECK-DAG: _ZGVdM4__Z3foov
//CHECK-DAG: _ZGVeM8__Z3foov

struct Object {
public:
  int a;
  int b;
  int c;
};

#pragma omp declare simd
Object* foo() {
  Object* o;
  o = new Object();
  o->a = 1;
  o->b = 2;
  o->c = 3;
  return o;
}
