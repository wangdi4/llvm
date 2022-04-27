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
// RUN:  -fopenmp-version=51 -fopenmp-late-outline -ast-dump %s \
// RUN:  | FileCheck %s --check-prefix=DUMP

// expected-no-diagnostics

#ifndef HEADER
#define HEADER

template <class T>
T foo(T t) {
  T v = T();
  T x = T();
  T expr = T();
  T e = T();
  T d = T();
  T r = T();

  #pragma omp atomic compare
  x = expr < x ? expr : x;

  #pragma omp atomic compare
  x = expr > x ? expr : x;

  #pragma omp atomic compare
  x = x < expr ? expr : x;

  #pragma omp atomic compare
  x = x > expr ? expr : x;

  #pragma omp atomic compare
  x = x == e ? d : x;

  #pragma omp atomic compare
  if (expr < x) { x = expr; }

  #pragma omp atomic compare
  if (expr > x) { x = expr; }

  #pragma omp atomic compare
  if (x < expr) { x = expr; }

  #pragma omp atomic compare
  if (x > expr) { x = expr; }

  #pragma omp atomic compare
  if (x == e) { x = d; }

  return T();
}

//PRINT:template<> float foo<float>(float t)
//PRINT:#pragma omp atomic compare
//PRINT:x = expr < x ? expr : x;
//PRINT:#pragma omp atomic compare
//PRINT:x = expr > x ? expr : x;
//PRINT:#pragma omp atomic compare
//PRINT:x = x < expr ? expr : x;
//PRINT:#pragma omp atomic compare
//PRINT:x = x > expr ? expr : x;
//PRINT:#pragma omp atomic compare
//PRINT:x = x == e ? d : x;
//PRINT:#pragma omp atomic compare
//PRINT:if (expr < x) {
//PRINT:    x = expr;
//PRINT:}
//PRINT:#pragma omp atomic compare
//PRINT:if (expr > x) {
//PRINT:    x = expr;
//PRINT:}
//PRINT:#pragma omp atomic compare
//PRINT:if (x < expr) {
//PRINT:    x = expr;
//PRINT:}
//PRINT:#pragma omp atomic compare
//PRINT:if (x > expr) {
//PRINT:    x = expr;
//PRINT:}
//PRINT:#pragma omp atomic compare
//PRINT:if (x == e) {
//PRINT:    x = d;
//PRINT:}
//DUMP:FunctionDecl{{.*}}foo 'float (float)'
//DUMP:OMPAtomicDirective
//DUMP:OMPCompareClause
//DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
//DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
//DUMP:ConditionalOperator {{.*}}'float':'float' lvalue
//DUMP:BinaryOperator {{.*}}'bool' '<'
//DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
//DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
//DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
//DUMP:DeclRefExpr {{.*}}'x' 'float':'float'

//PRINT:void bar() {
//PRINT:#pragma omp atomic compare
//PRINT:x = expr < x ? expr : x;
//PRINT:#pragma omp atomic compare
//PRINT:x = expr > x ? expr : x;
//PRINT:#pragma omp atomic compare
//PRINT:x = x < expr ? expr : x;
//PRINT:#pragma omp atomic compare
//PRINT:x = x > expr ? expr : x;
//PRINT:#pragma omp atomic compare
//PRINT:x = x == e ? d : x;
//PRINT:#pragma omp atomic compare
//PRINT:if (expr < x) {
//PRINT:    x = expr;
//PRINT:}
//PRINT:#pragma omp atomic compare
//PRINT:if (expr > x) {
//PRINT:    x = expr;
//PRINT:}
//PRINT:#pragma omp atomic compare
//PRINT:if (x < expr) {
//PRINT:    x = expr;
//PRINT:}
//PRINT:#pragma omp atomic compare
//PRINT:if (x > expr) {
//PRINT:    x = expr;
//PRINT:}
//PRINT:#pragma omp atomic compare
//PRINT:if (x == e) {
//PRINT:    x = d;
//PRINT:}
//DUMP:FunctionDecl {{.*}}bar 'void ()'
//DUMP:OMPAtomicDirective
//DUMP:OMPCompareClause
//DUMP:BinaryOperator {{.*}}'int' lvalue '='
//DUMP:DeclRefExpr {{.*}}'x' 'int'
//DUMP:ConditionalOperator {{.*}}'int' lvalue
//DUMP:BinaryOperator {{.*}}'bool' '<'
//DUMP:DeclRefExpr {{.*}}'expr' 'int'
//DUMP:DeclRefExpr {{.*}}'x' 'int'
//DUMP:DeclRefExpr {{.*}}'expr' 'int'
//DUMP:DeclRefExpr {{.*}}'x' 'int'

void bar() {
  int v = 0, x = 1, expr = 2, e = 3, d = 4, r = 5;

  #pragma omp atomic compare
  x = expr < x ? expr : x;

  #pragma omp atomic compare
  x = expr > x ? expr : x;

  #pragma omp atomic compare
  x = x < expr ? expr : x;

  #pragma omp atomic compare
  x = x > expr ? expr : x;

  #pragma omp atomic compare
  x = x == e ? d : x;

  #pragma omp atomic compare
  if (expr < x) { x = expr; }

  #pragma omp atomic compare
  if (expr > x) { x = expr; }

  #pragma omp atomic compare
  if (x < expr) { x = expr; }

  #pragma omp atomic compare
  if (x > expr) { x = expr; }

  #pragma omp atomic compare
  if (x == e) { x = d; }

  foo(float());
}

//PRINT:class A {
//DUMP:CXXRecordDecl {{.*}}class A definition
class A {
  int x = 1, expr = 2;
  int e = 2, d = 3;
  //DUMP:CXXMethodDecl {{.*}}bar 'void (){{.*}}'
  void bar() {
    //PRINT:#pragma omp atomic compare
    //PRINT:this->x = this->expr < this->x ? this->expr : this->x;
    //DUMP:OMPAtomicDirective
    //DUMP:OMPCompareClause
    //DUMP:BinaryOperator {{.*}}'='
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    #pragma omp atomic compare
    x = expr < x ? expr : x;

    //PRINT:#pragma omp atomic compare
    //PRINT:this->x = this->x == this->e ? this->d : this->x;
    //DUMP:OMPAtomicDirective
    //DUMP:OMPCompareClause
    //DUMP:BinaryOperator {{.*}}'='
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    #pragma omp atomic compare
    x = x == e ? d : x;

    //PRINT:#pragma omp atomic compare
    //PRINT:if (this->x > this->expr) {
    //PRINT:    this->x = this->expr;
    //PRINT:}
    //DUMP:OMPAtomicDirective
    //DUMP:OMPCompareClause
    //DUMP:IfStmt
    //DUMP:BinaryOperator {{.*}}'>'
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    #pragma omp atomic compare
    if (x > expr) { x = expr; }

    //PRINT:#pragma omp atomic compare
    //PRINT:if (this->x == this->e) {
    //PRINT:    this->x = this->d;
    //PRINT:}
    //DUMP:OMPAtomicDirective
    //DUMP:OMPCompareClause
    //DUMP:IfStmt
    //DUMP:BinaryOperator {{.*}}'=='
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    #pragma omp atomic compare
    if (x == e) { x = d; }
  }
};

//PRINT:void test() {
//DUMP:FunctionDecl {{.*}}test 'void ()'
void test()
{
  int x[4] = { 0,1,2,3};
  int expr = 1, e = 2, d = 3;
  //PRINT:#pragma omp atomic compare
  //PRINT:x[0] = expr < x[0] ? expr : x[0];
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:BinaryOperator {{.*}}'='
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 0
  #pragma omp atomic compare
  x[0] = expr < x[0] ? expr : x[0];

  //PRINT:#pragma omp atomic compare
  //PRINT:x[1] = x[1] == e ? d : x[1];
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:BinaryOperator {{.*}}'='
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 1
  #pragma omp atomic compare
  x[1] = x[1] == e ? d : x[1];

  //PRINT:#pragma omp atomic compare
  //PRINT:if (x[2] > expr) {
  //PRINT:    x[2] = expr;
  //PRINT:}
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'>'
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 2
  #pragma omp atomic compare
  if (x[2] > expr) { x[2] = expr; }

  //PRINT:#pragma omp atomic compare
  //PRINT:if (x[3] == e) {
  //PRINT:    x[3] = d;
  //PRINT:}
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'=='
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 3
  #pragma omp atomic compare
  if (x[3] == e) { x[3] = d; }
}

#endif // HEADER
// end INTEL_COLLAB
