// INTEL_COLLAB
//
// Ensures that a potentially throwing call is caught by a terminate
// handler inside the parallel region.
//
// RUN: %clang_cc1 -emit-llvm -o - -std=c++14 -fexceptions -fopenmp \
// RUN:  -fopenmp-late-outline -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck %s

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

extern "C" {
  extern double something (void);
}

//CHECK-LABEL: zap
void zap()
{
  //CHECK: call token{{.*}}DIR.OMP.PARALLEL
  //CHECK: invoke double @something
  //CHECK-NEXT: unwind label %[[TLP_Z:terminate.lpad[0-9]*]]
  //CHECK: region.exit{{.*}}DIR.OMP.END.PARALLEL
  #pragma omp parallel
  {
    something();
  }
  //CHECK: [[TLP_Z]]:
}
// end INTEL_COLLAB
