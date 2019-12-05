// INTEL_COLLAB
//
// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -disable-llvm-passes \
// RUN:  -fexceptions -fcxx-exceptions \
// RUN:  -fopenmp -fintel-compatibility -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu \
// RUN:  | FileCheck %s --check-prefixes=BOTH,NOOPT

// Same as above adding -O2
// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -disable-llvm-passes -O2 \
// RUN:  -fexceptions -fcxx-exceptions \
// RUN:  -fopenmp -fintel-compatibility -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu \
// RUN:  | FileCheck %s -check-prefixes=BOTH,OPT

extern void bar(float*);
extern void goo(float*);

// Optimized and non-optimized IR is different, so test both.
// Loops and non-loops use a different code path, so test both.

//BOTH: test_loops
void test_loops(float *x)
{
  //BOTH: call token{{.*}}DIR.OMP.PARALLEL.LOOP
  //BOTH: invoke void{{.*}}bar
  //BOTH-NEXT: unwind label %[[TLP:.*lpad[0-9]*]]
  #pragma omp parallel for
  for (int i = 0; i < 10; ++i)
    try { bar(x); } catch(...) {}
  //OPT: [[TLP]]:
  //OPT: br label %[[TH1:terminate.handler.*]]

  //BOTH: call token{{.*}}DIR.OMP.PARALLEL.LOOP
  //BOTH: invoke void{{.*}}goo
  //BOTH-NEXT: unwind label %[[TLP20:.*lpad[0-9]*]]
  #pragma omp parallel for
  for (int i = 0; i < 10; ++i)
    try { goo(x); } catch(...) {}

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


//BOTH: test_non_loops
void test_non_loops(float *x)
{
  //BOTH: call token{{.*}}DIR.OMP.PARALLEL
  //BOTH: invoke void{{.*}}bar
  //BOTH-NEXT: unwind label %[[TLP:.*lpad[0-9]*]]
  #pragma omp parallel
  try { bar(x); } catch(...) {}

  //BOTH: call token{{.*}}DIR.OMP.PARALLEL
  //BOTH: invoke void{{.*}}goo
  //BOTH-NEXT: unwind label %[[TLP20:.*lpad[0-9]*]]
  #pragma omp parallel
  try { goo(x); } catch(...) {}

  //BOTH: [[TLP]]:
  //BOTH: call void @__clang_call_terminate
  //BOTH: [[TLP20]]:
  //BOTH: call void @__clang_call_terminate
}

// Since it is not legal to throw out of a OpenMP region,
// don't use invokes on calls that are not nested in a try block.

struct A { A(); ~A(); };
extern void may_throw();

// BOTH-LABEL: call_invoke_check
void call_invoke_check()
{
  A a1;
  //BOTH: invoke void{{.*}}may_throw
  may_throw();
  //BOTH: call token{{.*}}DIR.OMP.PARALLEL.LOOP
  #pragma omp parallel for
  for (int i = 0; i < 100; ++i)
  {
    //BOTH: call void{{.*}}may_throw
    may_throw();
    try {
      //BOTH: invoke void{{.*}}may_throw
      may_throw();
    } catch(...) {}
    //BOTH: call void{{.*}}may_throw
    may_throw();
  }
  //BOTH: call {{.*}}DIR.OMP.END.PARALLEL.LOOP
  //BOTH: invoke void{{.*}}may_throw
  may_throw();
}

// BOTH-LABEL: call_invoke_check_two
void call_invoke_check_two() {
  try {
    //BOTH: call token{{.*}}DIR.OMP.TARGET
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 0; i < 100; ++i)
      for (int j = 0; j < 100; ++j) {
        //BOTH: call void{{.*}}may_throw
        may_throw();
    }
    //BOTH: call {{.*}}DIR.OMP.END.TARGET
    //BOTH: invoke void{{.*}}may_throw
    may_throw();
  } catch(...) { }
}

// end INTEL_COLLAB
