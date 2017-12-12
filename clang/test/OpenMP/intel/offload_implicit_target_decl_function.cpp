// RUN: %clang_cc1 -verify -fopenmp -fopenmp-targets=csa -ast-dump %s | FileCheck %s

// expected-no-diagnostics
void foo() {
  #pragma omp target nowait
  {}
}

//CHECK: foo
//CHECK: OMPDeclareTargetDeclAttr{{.*}}Implicit
