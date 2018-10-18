// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fexceptions -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s

extern "C" void abort (void) throw () __attribute__ ((__noreturn__));
extern "C" void exit (int __status) throw () __attribute__ ((__noreturn__));

void bar(int);

[[noreturn]] void notcomingback(){ abort(); }

//CHECK-LABEL: @_Z3foov
void foo()
{
//CHECK: DIR.OMP.PARALLEL
//CHECK: DIR.OMP.END.PARALLEL
  #pragma omp parallel
  {
    bar(111);
    notcomingback();
    bar(222);
  }
}

//CHECK-LABEL: @_Z4foo1v
void foo1()
{
//CHECK: DIR.OMP.PARALLEL
//CHECK: DIR.OMP.END.PARALLEL
  #pragma omp parallel
  {
    bar(111);
    exit(-1);
    bar(222);
  }
}
