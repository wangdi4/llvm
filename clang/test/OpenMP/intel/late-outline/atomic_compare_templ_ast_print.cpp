// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -ast-print %s \
// RUN:  -fopenmp-late-outline | FileCheck %s --check-prefix=PRINT

// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -ast-dump %s \
// RUN:  -fopenmp-late-outline | FileCheck %s --check-prefix=DUMP

// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -fopenmp-late-outline \
// RUN:  -emit-pch -o %t.pch %s

// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -ast-print \
// RUN:  -fopenmp-late-outline -include-pch %t.pch %s \
// RUN:  | FileCheck %s --check-prefix=PRINT

// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -ast-dump-all \
// RUN:  -fopenmp-late-outline -include-pch %t.pch %s \
// RUN:  | FileCheck %s --check-prefix=DUMP

// RUN: %clang_cc1 -verify -triple i386-pc-win32 -fopenmp \
// RUN:  -fopenmp-late-outline -fopenmp-version=51 -ast-dump %s \
// RUN:  | FileCheck %s --check-prefix=DUMP

// expected-no-diagnostics

#ifndef HEADER
#define HEADER

struct S { int i; };
constexpr bool inline operator<(const S& A, const S& B) { return A.i < B.i; }
constexpr bool inline operator>(const S& A, const S& B) { return A.i > B.i; }
constexpr bool inline operator==(const S& A, const S& B) { return A.i == B.i; }

//PRINT:template <typename T> inline void atomic_template(T *x, T expr, T e, T d) {
//DUMP:FunctionTemplateDecl {{.*}}atomic_template
template <typename T>
inline void atomic_template(T *x, T expr, T e, T d)
{
//PRINT:#pragma omp atomic compare
//PRINT:    if (*x < expr) {
//PRINT:        *x = expr;
//PRINT:    }
//PRINT:#pragma omp atomic compare
//PRINT:    if (*x > expr) {
//PRINT:        *x = expr;
//PRINT:    }
//PRINT:#pragma omp atomic compare
//PRINT:    if (*x == e) {
//PRINT:        *x = d;
//PRINT:    }
//PRINT:#pragma omp atomic compare
//PRINT:    *x = expr < *x ? expr : *x;
//PRINT:#pragma omp atomic compare
//PRINT:    *x = *x > expr ? expr : *x;
//PRINT:#pragma omp atomic compare
//PRINT:    *x = *x == e ? d : *x;

//DUMP:OMPAtomicDirective
//DUMP:OMPCompareClause
//DUMP:CXXOperatorCallExpr {{.*}}'<dependent type>' '<'
  #pragma omp atomic compare
  if (*x < expr) { *x = expr; }

//DUMP:OMPAtomicDirective
//DUMP:OMPCompareClause
//DUMP:CXXOperatorCallExpr {{.*}}'<dependent type>' '>'
  #pragma omp atomic compare
  if (*x > expr) { *x = expr; }

//DUMP:OMPAtomicDirective
//DUMP:OMPCompareClause
//DUMP:CXXOperatorCallExpr {{.*}}'<dependent type>' '=='
  #pragma omp atomic compare
  if (*x == e) { *x = d; }

//DUMP:OMPAtomicDirective
//DUMP:OMPCompareClause
//DUMP:CXXOperatorCallExpr {{.*}}'<dependent type>' '<'
  #pragma omp atomic compare
  *x = expr < *x ? expr : *x;

//DUMP:OMPAtomicDirective
//DUMP:OMPCompareClause
//DUMP:CXXOperatorCallExpr {{.*}}'<dependent type>' '>'
  #pragma omp atomic compare
  *x = *x > expr ? expr : *x;

//DUMP:OMPAtomicDirective
//DUMP:OMPCompareClause
//DUMP:CXXOperatorCallExpr {{.*}}'<dependent type>' '=='
  #pragma omp atomic compare
  *x = *x == e ? d : *x;
}

//PRINT:template<> inline void atomic_template<int>(int *x, int expr, int e, int d) {
//DUMP:FunctionDecl {{.*}}atomic_template

//PRINT:#pragma omp atomic compare
//PRINT:    if (*x < expr) {
//PRINT:        *x = expr;
//PRINT:    }
//PRINT:#pragma omp atomic compare
//PRINT:    if (*x > expr) {
//PRINT:        *x = expr;
//PRINT:    }
//PRINT:#pragma omp atomic compare
//PRINT:    if (*x == e) {
//PRINT:        *x = d;
//PRINT:    }
//PRINT:#pragma omp atomic compare
//PRINT:    *x = expr < *x ? expr : *x;
//PRINT:#pragma omp atomic compare
//PRINT:    *x = *x > expr ? expr : *x;
//PRINT:#pragma omp atomic compare
//PRINT:    *x = *x == e ? d : *x;

//DUMP:OMPAtomicDirective
//DUMP:OMPCompareClause
//DUMP:BinaryOperator {{.*}}'bool' '<'
//
//DUMP:OMPAtomicDirective
//DUMP:OMPCompareClause
//DUMP:BinaryOperator {{.*}}'bool' '>'

//DUMP:OMPAtomicDirective
//DUMP:OMPCompareClause
//DUMP:BinaryOperator {{.*}}'bool' '=='

//DUMP:OMPAtomicDirective
//DUMP:OMPCompareClause
//DUMP:BinaryOperator {{.*}}'bool' '<'

//DUMP:OMPAtomicDirective
//DUMP:OMPCompareClause
//DUMP:BinaryOperator {{.*}}'bool' '>'

//DUMP:OMPAtomicDirective
//DUMP:OMPCompareClause
//DUMP:BinaryOperator {{.*}}'bool' '=='

//PRINT:void foo() {
//DUMP:FunctionDecl {{.*}}foo
void foo()
{
  int x, expr = 10, e = 20, d = 30;
  atomic_template(&x, expr, e, d);
}
#endif // HEADER
// end INTEL_COLLAB
