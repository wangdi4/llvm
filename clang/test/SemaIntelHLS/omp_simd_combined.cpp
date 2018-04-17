//RUN: %clang_cc1 -Wpragmas -fhls -fsyntax-only -fopenmp -fintel-compatibility -fintel-openmp-region -ast-dump -verify -pedantic %s | FileCheck %s
//expected-no-diagnostics

// Test that applicable loop pragmas work with pragma omp simd.
// Note: At this time we don't have a specification describing which of
// our added loop pragmas make sense with OpenMP pragmas.

//CHECK: FunctionDecl{{.*}}foo
void foo()
{
  //CHECK: OMPSimdDirective
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}Unroll Enable
  #pragma unroll
  #pragma omp simd simdlen(16)
  for (int i=0;i<32;++i) {}

  //CHECK: OMPSimdDirective
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}Unroll Enable
  #pragma omp simd simdlen(16)
  #pragma unroll
  for (int i=0;i<32;++i) {}

  //CHECK: OMPSimdDirective
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}IVDep Enable
  #pragma omp simd simdlen(16)
  #pragma ivdep
  for (int i=0;i<32;++i) {}

  //CHECK: OMPSimdDirective
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}IVDep Enable
  #pragma ivdep
  #pragma omp simd simdlen(16)
  for (int i=0;i<32;++i) {}
}
