//RUN: %clang_cc1 -Wpragmas -fhls -fsyntax-only -fopenmp -fintel-compatibility -fintel-openmp-region -ast-dump -verify -pedantic %s | FileCheck %s
//expected-no-diagnostics

// Test that applicable loop pragmas work with pragma omp simd.
// Note: At this time we don't have a specification describing which of
// our added loop pragmas make sense with OpenMP pragmas.

//CHECK: FunctionDecl{{.*}}foo
void foo()
{
  // Clang LoopHint pragmas must attach directly to the loop to work
  // properly.  When using OpenMP pragmas specify them before the
  // loop pragmas.

  //CHECK: OMPSimdDirective
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}Unroll Enable
  #pragma omp simd simdlen(16)
  #pragma unroll
  for (int i=0;i<32;++i) {}

  //CHECK: OMPSimdDirective
  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelFPGAIVDepAttr{{.*}} 0
  #pragma omp simd simdlen(16)
  #pragma ivdep
  for (int i=0;i<32;++i) {}
}
