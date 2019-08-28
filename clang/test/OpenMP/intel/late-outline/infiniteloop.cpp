// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -std=c++14 -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s


void bar(int);


//CHECK-LABEL: @_Z3foov
void foo()
{
//CHECK: DIR.OMP.PARALLEL
//CHECK: DIR.OMP.END.PARALLEL
  #pragma omp parallel
  {
L:
    bar(111);
    goto L;
    bar(222);
  }
}

class A
{
  public:
  ~A();
};

//CHECK-LABEL: @_Z4foo1v
void foo1()
{
  A obj;
//CHECK: DIR.OMP.PARALLEL
//CHECK: DIR.OMP.END.PARALLEL
  #pragma omp parallel
  {
    A obj;
L:
    bar(111);
    goto L;
    bar(222);
  }
}
// end INTEL_COLLAB
