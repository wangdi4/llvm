// INTEL_COLLAB
//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc \
//RUN:  -disable-llvm-passes -fopenmp -fopenmp-targets=spir64    \
//RUN:  -fopenmp-late-outline -I%S/Inputs -verify -DNOWARN       \
//RUN:  -Wsource-uses-openmp -o %t_host1.bc %s

//RUN: %clang_cc1 -triple spir64 -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 -fopenmp-late-outline   \
//RUN:  -fopenmp-is-device -verify -I%S/Inputs                   \
//RUN:  -fopenmp-host-ir-file-path %t_host1.bc  -o %t-ir1.txt %s

//RUN: %clang_cc1 -triple spir64 -emit-llvm -disable-llvm-passes -fopenmp \
//RUN:  -fopenmp-targets=spir64 -fopenmp-late-outline -I%S/Inputs         \
//RUN:  -fopenmp-is-device -verify -DNOWARN -Wno-unsupported-omp-api      \
//RUN:  -fopenmp-host-ir-file-path %t_host1.bc -o %t-ir2.txt %s

//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc \
//RUN:  -disable-llvm-passes -fopenmp -fopenmp-targets=x86_64    \
//RUN:  -fopenmp-late-outline -I%S/Inputs -verify -DNOWARN       \
//RUN:  -Wsource-uses-openmp -o %t_host2.bc %s

//RUN: %clang_cc1 -triple x86_64 -emit-llvm -disable-llvm-passes -fopenmp \
//RUN:  -fopenmp-targets=x86_64 -fopenmp-late-outline -DNOWARN            \
//RUN:  -fopenmp-is-device -verify -I%S/Inputs                            \
//RUN:  -fopenmp-host-ir-file-path %t_host2.bc -o %t-ir3.txt %s

#ifdef NOWARN
// expected-no-diagnostics
#endif

#include <omp.h>

double foo1()
{
  double f1d = omp_get_wtime();
  return f1d;
}

double foo2()
{
#ifndef NOWARN
  //expected-warning@+2 {{call to 'omp_get_wtime' is not supported for target 'spir64'}}
#endif
  double f2d = omp_get_wtime();
  return f2d;
}

void bar(const char *, ...);
int main() {

  double a = omp_get_wtime();;
  foo1();

  #pragma omp target
  {
#ifndef NOWARN
    //expected-warning@+2 {{call to 'omp_get_wtime' is not supported for target 'spir64'}}
#endif
    a = omp_get_wtime();
    foo2();
  }
  return 0;
}
// end INTEL_COLLAB
