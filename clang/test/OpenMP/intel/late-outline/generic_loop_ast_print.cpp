// INTEL_COLLAB
// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-late-outline -fopenmp-version=51 \
// RUN:   -fsyntax-only -verify %s

// expected-no-diagnostics

// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-late-outline -fopenmp-version=51 \
// RUN:   -ast-print %s | FileCheck %s --check-prefix=PRINT

// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-late-outline -fopenmp-version=51 \
// RUN:   -ast-dump  %s | FileCheck %s --check-prefix=DUMP

// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-late-outline -fopenmp-version=51 \
// RUN:   -emit-pch -o %t %s

// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-late-outline -fopenmp-version=51 \
// RUN:   -include-pch %t -ast-dump-all %s | FileCheck %s --check-prefix=DUMP

// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-late-outline -fopenmp-version=51 \
// RUN:   -include-pch %t -ast-print %s | FileCheck %s --check-prefix=PRINT

#ifndef HEADER
#define HEADER

//PRINT: int test() {
//DUMP: FunctionDecl {{.*}}test 'int ()'
int test() {
  constexpr int N = 100;
  float MTX[N][N];
  //PRINT: #pragma omp target teams distribute parallel for map(tofrom: MTX)
  //PRINT: #pragma omp loop
  //DUMP: OMPTargetTeamsDistributeParallelForDirective
  //DUMP: CapturedStmt
  //DUMP: ForStmt
  //DUMP: CompoundStmt
  //DUMP: OMPGenericLoopDirective
  #pragma omp target teams distribute parallel for map(MTX)
  for (auto i = 0; i < N; ++i) {
    #pragma omp loop
    for (auto j = 0; j < N; ++j) {
      MTX[i][j] = 0;
    }
  }
  return 0;
}

#endif // HEADER
// end INTEL_COLLAB
