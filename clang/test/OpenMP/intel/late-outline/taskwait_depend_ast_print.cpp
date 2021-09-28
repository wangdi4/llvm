// INTEL_COLLAB
// expected-no-diagnostics

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-late-outline \
//RUN:   -fopenmp-version=51 -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                       \
//RUN:   -ast-print %s | FileCheck %s --check-prefix=PRINT

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-late-outline \
//RUN:   -fopenmp-version=51 -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                       \
//RUN:   -ast-dump %s | FileCheck %s --check-prefix=DUMP

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-late-outline \
//RUN:   -fopenmp-version=51 -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                       \
//RUN:   -emit-pch -o %t %s

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-late-outline \
//RUN:   -fopenmp-version=51 -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                       \
//RUN:   -include-pch %t -ast-print %s | FileCheck %s --check-prefix=PRINT

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-late-outline \
//RUN:   -fopenmp-version=51 -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                       \
//RUN:   -include-pch %t -ast-dump-all %s | FileCheck %s --check-prefix=DUMP

#ifndef HEADER
#define HEADER

void bar();

void foo()
{
  int var = 1;
  int arr[100] = {0};

  #pragma omp task
  {
    bar();
    #pragma omp taskwait depend(in:var)
    #pragma omp taskwait depend(out:var) nowait
    #pragma omp taskwait nowait depend(inout:arr)
    #pragma omp taskwait depend(in:var) nowait depend(out:var) \
                         depend(inout:arr)
  }
}

//DUMP: FunctionDecl {{.*}} foo 'void ()'
//DUMP: OMPTaskDirective {{.*}}
//DUMP: OMPTaskwaitDirective {{.*}} openmp_standalone_directive
//DUMP: OMPDependClause {{.*}}
//DUMP: DeclRefExpr {{.*}} 'int' lvalue Var {{.*}} 'var' 'int'
//DUMP: OMPTaskwaitDirective {{.*}} openmp_standalone_directive
//DUMP: OMPDependClause
//DUMP: DeclRefExpr {{.*}} 'int' lvalue Var {{.*}} 'var' 'int'
//DUMP: OMPNowaitClause
//DUMP: OMPTaskwaitDirective {{.*}} openmp_standalone_directive
//DUMP: OMPNowaitClause
//DUMP: OMPDependClause
//DUMP: DeclRefExpr {{.*}} 'int [100]' lvalue Var {{.*}} 'arr' 'int [100]'
//DUMP: OMPTaskwaitDirective {{.*}} openmp_standalone_directive
//DUMP: OMPDependClause
//DUMP: DeclRefExpr {{.*}} 'int' lvalue Var {{.*}} 'var' 'int'
//DUMP: OMPNowaitClause
//DUMP: OMPDependClause
//DUMP: DeclRefExpr {{.*}} 'int' lvalue Var {{.*}} 'var' 'int'
//DUMP: OMPDependClause
//DUMP: DeclRefExpr {{.*}} 'int [100]' lvalue Var {{.*}} 'arr' 'int [100]'
//PRINT: #pragma omp taskwait depend(in : var)
//PRINT: #pragma omp taskwait depend(out : var) nowait
//PRINT: #pragma omp taskwait nowait depend(inout : arr)
//PRINT: #pragma omp taskwait depend(in : var) nowait depend(out : var) depend(inout : arr)
#endif // HEADER
// end INTEL_COLLAB
