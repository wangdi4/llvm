// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fexceptions -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s
extern void bar(float*);
extern void goo(float*);

//CHECK-LABEL: foo
void foo(float *x)
{
  //CHECK: call token{{.*}}DIR.OMP.PARALLEL.LOOP
  //CHECK: invoke void{{.*}}bar
  //CHECK-NEXT: unwind label %[[TLP:terminate.lpad[0-9]*]]
  #pragma omp parallel for
  for (int i = 0; i < 10; ++i)
    bar(x);

  //CHECK: call token{{.*}}DIR.OMP.PARALLEL.LOOP
  //CHECK: invoke void{{.*}}goo
  //CHECK-NEXT: unwind label %[[TLP20:terminate.lpad[0-9]*]]
  #pragma omp parallel for
  for (int i = 0; i < 10; ++i)
    goo(x);

  //CHECK: [[TLP]]:
  //CHECK: [[TLP20]]:
}
