// INTEL_COLLAB
// expected-no-diagnostics

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
//RUN:   -x c++ -std=c++14 -fexceptions -fcxx-exceptions                   \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                       \
//RUN:   -ast-print %s | FileCheck %s --check-prefix=PRINT

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
//RUN:   -x c++ -std=c++14 -fexceptions -fcxx-exceptions                   \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                       \
//RUN:   -ast-dump %s | FileCheck %s --check-prefix=DUMP

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
//RUN:   -x c++ -std=c++14 -fexceptions -fcxx-exceptions                   \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                       \
//RUN:   -emit-pch -o %t %s

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
//RUN:   -x c++ -std=c++14 -fexceptions -fcxx-exceptions                   \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                       \
//RUN:   -include-pch %t -ast-print %s | FileCheck %s --check-prefix=PRINT

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
//RUN:   -x c++ -std=c++14 -fexceptions -fcxx-exceptions                   \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                       \
//RUN:   -include-pch %t -ast-dump-all %s | FileCheck %s --check-prefix=DUMP

#ifndef HEADER
#define HEADER

int main() {
  int a;
  int b;
  int *p;
  int p2[30];
  int *p3;
  int *p4;

  #pragma ompx prefetch data(1:p[0:10]) data(2:p2[3:20],p3[5:5]) data(p4[4:99]) if (a < b)
  return 0;
}
//DUMP: FunctionDecl{{.*}} main 'int ()'
//DUMP: OMPPrefetchDirective
//DUMP: OMPDataClause
//DUMP: OMPArraySectionExpr
//DUMP: DeclRefExpr {{.*}}'p' 'int *'
//DUMP: IntegerLiteral {{.*}}'int' 0
//DUMP: IntegerLiteral {{.*}}'int' 10
//DUMP: OMPDataClause
//DUMP: OMPArraySectionExpr
//DUMP: DeclRefExpr {{.*}}'p2' 'int[30]'
//DUMP: IntegerLiteral {{.*}}'int' 3
//DUMP: IntegerLiteral {{.*}}'int' 20
//DUMP: OMPArraySectionExpr
//DUMP: DeclRefExpr {{.*}}'p3' 'int *'
//DUMP: IntegerLiteral {{.*}}'int' 5
//DUMP: IntegerLiteral {{.*}}'int' 5
//DUMP: OMPDataClause
//DUMP: OMPArraySectionExpr
//DUMP: DeclRefExpr {{.*}}'p4' 'int *'
//DUMP: IntegerLiteral {{.*}}'int' 4
//DUMP: IntegerLiteral {{.*}}'int' 99
//DUMP: OMPIfClause
//DUMP: BinaryOperator{{.*}}'bool' '<'
//DUMP: ImplicitCastExpr{{.*}}'int' <LValueToRValue>
//DUMP: DeclRefExpr{{.*}}'int' lvalue Var{{.*}}'a' 'int'
//DUMP: ImplicitCastExpr{{.*}}'int' <LValueToRValue>
//DUMP: DeclRefExpr{{.*}}'int' lvalue Var{{.*}}'b' 'int'
//PRINT: #pragma ompx prefetch data(1: p[0:10]) data(2: p2[3:20],p3[5:5]) data(p4[4:99]) if(a < b)

template <typename T, unsigned hint, unsigned size>
T run() {
  T foo[size];
  #pragma ompx prefetch data(hint:foo[0:size])

  return foo[0];
}

int template_test() {
  double d;
  d = run<double,1,10>();
  return 0;
}

//DUMP: FunctionTemplateDecl {{.*}}run
//DUMP: TemplateTypeParmDecl {{.*}}typename depth 0 index 0 T
//DUMP: NonTypeTemplateParmDecl {{.*}}'unsigned int' depth 0 index 1 hint
//DUMP: NonTypeTemplateParmDecl {{.*}}'unsigned int' depth 0 index 2 size
//DUMP: FunctionDecl {{.*}}run 'T ()'
//DUMP: OMPPrefetchDirective {{.*}}openmp_standalone_directive
//DUMP: OMPDataClause
//DUMP: OMPArraySectionExpr {{.*}}'<dependent type>' lvalue
//DUMP: DeclRefExpr {{.*}}'foo' 'T[size]'
//DUMP: IntegerLiteral {{.*}}'int' 0
//DUMP: DeclRefExpr {{.*}}NonTypeTemplateParm {{.*}}'size' 'unsigned int'
//PRINT: #pragma ompx prefetch data(hint: foo[0:size])

//DUMP: FunctionDecl {{.*}}run 'double ()'
//DUMP: TemplateArgument type 'double'
//DUMP: BuiltinType {{.*}} 'double'
//DUMP: TemplateArgument integral 1
//DUMP: TemplateArgument integral 10
//DUMP: OMPPrefetchDirective {{.*}}openmp_standalone_directive
//DUMP: OMPDataClause
//DUMP: OMPArraySectionExpr {{.*}}<OpenMP array section type>' lvalue
//DUMP: DeclRefExpr {{.*}}'foo' 'double[10]'
//DUMP: IntegerLiteral {{.*}}'int' 0
//DUMP: SubstNonTypeTemplateParmExpr {{.*}}'unsigned int'
//DUMP: NonTypeTemplateParmDecl {{.*}}'unsigned int' depth 0 index 2 size
//DUMP: IntegerLiteral {{.*}}'unsigned int' 10
//PRINT: #pragma ompx prefetch data(1U: foo[0:10U])
#endif // HEADER
// end INTEL_COLLAB
