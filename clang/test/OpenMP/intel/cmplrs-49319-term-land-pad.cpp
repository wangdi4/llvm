// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fexceptions -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu \
// RUN:  | FileCheck %s --check-prefixes=BOTH,NOOPT
// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -disable-llvm-passes -O2 \
// RUN:  -fexceptions -fopenmp -fintel-compatibility -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu \
// RUN:  | FileCheck %s -check-prefixes=BOTH,OPT

extern void bar(float*);
extern void goo(float*);

//CHECK-BOTH: foo
void foo(float *x)
{
  //BOTH: call token{{.*}}DIR.OMP.PARALLEL.LOOP
  //BOTH: invoke void{{.*}}bar
  //BOTH-NEXT: unwind label %[[TLP:.*lpad[0-9]*]]
  #pragma omp parallel for
  for (int i = 0; i < 10; ++i)
    bar(x);
  //OPT: [[TLP]]:
  //OPT: br label %[[TH1:terminate.handler.*]]

  //BOTH: call token{{.*}}DIR.OMP.PARALLEL.LOOP
  //BOTH: invoke void{{.*}}goo
  //BOTH-NEXT: unwind label %[[TLP20:.*lpad[0-9]*]]
  #pragma omp parallel for
  for (int i = 0; i < 10; ++i)
    goo(x);

  //OPT: [[TH1]]:
  //OPT: call void @__clang_call_terminate

  //NOOPT: [[TLP]]:
  //NOOPT: call void @__clang_call_terminate
  //NOOPT: [[TLP20]]:
  //NOOPT: call void @__clang_call_terminate

  //OPT: [[TLP20]]:
  //OPT: br label %[[TH2:terminate.handler.*]]
  //OPT: [[TH2]]:
  //OPT: call void @__clang_call_terminate
}

//CHECK-BOTH: otherfoo
void otherfoo(float *x)
{
  //BOTH: call token{{.*}}DIR.OMP.PARALLEL
  //BOTH: invoke void{{.*}}bar
  //BOTH-NEXT: unwind label %[[TLP:.*lpad[0-9]*]]
  #pragma omp parallel
  bar(x);

  //BOTH: call token{{.*}}DIR.OMP.PARALLEL
  //BOTH: invoke void{{.*}}goo
  //BOTH-NEXT: unwind label %[[TLP20:.*lpad[0-9]*]]
  #pragma omp parallel
  goo(x);

  //BOTH: [[TLP]]:
  //BOTH: call void @__clang_call_terminate
  //BOTH: [[TLP20]]:
  //BOTH: call void @__clang_call_terminate
}
