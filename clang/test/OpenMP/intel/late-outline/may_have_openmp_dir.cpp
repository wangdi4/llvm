// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

void bar();
// foo1,foo2,foo3 should have the same attributes
//CHECK: define{{.*}}void @_Z4foo1v() #[[FOO1:[0-9]+]]
void foo1() {
  // CHECK: DIR.OMP.PARALLEL
  #pragma omp parallel
  {
    bar();
  }
}

//CHECK: define{{.*}}void @_Z4foo2v() #[[FOO1]]
void foo2() {
  // CHECK: DIR.OMP.PARALLEL
  #pragma omp parallel for
  for (int i=0; i<10; ++i) {
    bar();
  }
}

//CHECK: define{{.*}}void @_Z4foo3v() #[[FOO1]]
void foo3() {
  // CHECK: DIR.OMP.PARALLEL
  #pragma omp parallel for
  for (int i=0; i<10; ++i) {
    bar();
  }
  #pragma omp parallel
  {
    bar();
  }
  foo2();
}

//CHECK: define{{.*}}void @_Z4foo4v() #[[FOO4:[0-9]+]]
void foo4() {}

//CHECK: attributes #[[FOO1]] = {{.*}}"may-have-openmp-directive"="true"
//CHECK-NOT: attributes #[[FOO4]] = {{.*}}"may-have-openmp-directive"="true"
// end INTEL_COLLAB
