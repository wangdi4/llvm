// INTEL_COLLAB
// expected-no-diagnostics

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp-late-outline \
//RUN:   -fopenmp -fopenmp-version=52 \
//RUN:   -x c++ -std=c++14 -fexceptions -fcxx-exceptions                   \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                       \
//RUN:   -ast-print %s | FileCheck %s --check-prefix=PRINT

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp-late-outline \
//RUN:   -fopenmp -fopenmp-version=52 \
//RUN:   -x c++ -std=c++14 -fexceptions -fcxx-exceptions                   \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                       \
//RUN:   -ast-dump %s | FileCheck %s --check-prefix=DUMP

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp-late-outline \
//RUN:   -fopenmp -fopenmp-version=52 \
//RUN:   -x c++ -std=c++14 -fexceptions -fcxx-exceptions                   \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                       \
//RUN:   -emit-pch -o %t %s

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp-late-outline \
//RUN:   -fopenmp -fopenmp-version=52 \
//RUN:   -x c++ -std=c++14 -fexceptions -fcxx-exceptions                   \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                       \
//RUN:   -include-pch %t -ast-print %s | FileCheck %s --check-prefix=PRINT

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp-late-outline \
//RUN:   -fopenmp -fopenmp-version=52 \
//RUN:   -x c++ -std=c++14 -fexceptions -fcxx-exceptions                   \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                       \
//RUN:   -include-pch %t -ast-dump-all %s | FileCheck %s --check-prefix=DUMP

#ifndef HEADER
#define HEADER
int foo1() {
  int a;
  int i = 1, j=2;
  char v1 = 0;
  #pragma omp scope private(a) firstprivate(j, v1) reduction(+:i) allocate(v1) nowait
  { 
    a = 123; 
    ++i; 
    j++;
  }
  return i;
}

//DUMP: FunctionDecl {{.*}}foo1 'int ()'
//DUMP: OMPScopeDirective
//DUMP: OMPPrivateClause
//DUMP: DeclRefExpr {{.*}}'int' lvalue Var{{.*}}'a' 'int'
//DUMP: OMPFirstprivateClause
//DUMP: DeclRefExpr {{.*}}'int' lvalue Var{{.*}}'j' 'int'
//DUMP: DeclRefExpr {{.*}}'char' lvalue Var {{.*}}'v1' 'char'
//DUMP: OMPReductionClause
//DUMP: DeclRefExpr {{.*}}'int' lvalue Var{{.*}}'i' 'int'
//DUMP: OMPAllocateClause
//DUMP: DeclRefExpr {{.*}}'char' lvalue Var {{.*}}'v1' 'char'
//DUMP: OMPNowaitClause
//PRINT: #pragma omp scope private(a) firstprivate(j,v1) reduction(+: i) allocate(v1) nowait

template <typename T>
T run() {
  T a;
  T b;
  T c;
  T v1;

  #pragma omp scope private(a) firstprivate(c, v1) allocate(v1) reduction(*:b)
  { 
    b *= a; 
    c = a;
    v1 = a;
  }
  return b;
}

int template_test() {
  double d;
  d = run<double>();
  return 0;
}

//DUMP: FunctionTemplateDecl {{.*}}run
//DUMP: TemplateTypeParmDecl {{.*}}referenced typename depth 0 index 0 T
//DUMP: FunctionDecl {{.*}}run 'T ()'
//DUMP: OMPScopeDirective
//DUMP: OMPPrivateClause
//DUMP: DeclRefExpr {{.*}}'T' lvalue Var {{.*}} 'a' 'T'
//DUMP: OMPFirstprivateClause
//DUMP: DeclRefExpr {{.*}}'T' lvalue Var {{.*}} 'c' 'T'
//DUMP: DeclRefExpr {{.*}}'T' lvalue Var {{.*}}'v1' 'T'
//DUMP: OMPAllocateClause
//DUMP: DeclRefExpr {{.*}}'T' lvalue Var {{.*}}'v1' 'T'
//DUMP: OMPReductionClause
//DUMP: DeclRefExpr {{.*}}'T' lvalue Var {{.*}} 'b' 'T'
//DUMP: FunctionDecl {{.*}}used run 'double ()'
//DUMP: TemplateArgument type 'double'
//DUMP: BuiltinType {{.*}}'double'
//DUMP: OMPScopeDirective
//DUMP: OMPPrivateClause
//DUMP: DeclRefExpr {{.*}}'double' lvalue Var {{.*}} 'a' 'double'
//DUMP: OMPFirstprivateClause
//DUMP: DeclRefExpr {{.*}}'double' lvalue Var {{.*}} 'c' 'double'
//DUMP: DeclRefExpr {{.*}}'double' lvalue Var {{.*}}'v1' 'double'
//DUMP: OMPAllocateClause
//DUMP: DeclRefExpr {{.*}}'double' lvalue Var {{.*}}'v1' 'double'
//DUMP: OMPReductionClause
//DUMP: DeclRefExpr {{.*}}'double' lvalue Var {{.*}} 'b' 'double'
//PRINT: #pragma omp scope private(a) firstprivate(c,v1) allocate(v1) reduction(*: b)
#endif // HEADER
// end INTEL_COLLAB
