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

  #pragma ompx prefetch data(p:1:10, &b:2:20) if (a < b)
  return 0;
}
//DUMP: FunctionDecl{{.*}} main 'int ()'
//DUMP: OMPPrefetchDirective
//DUMP: OMPDataClause
//DUMP: DeclRefExpr {{.*}} lvalue Var{{.*}}'p' 'int *'
//DUMP: IntegerLiteral {{.*}}'int' 1
//DUMP: IntegerLiteral {{.*}}'int' 10
//DUMP: DeclRefExpr {{.*}}'int' lvalue Var {{.*}} 'b' 'int'
//DUMP: IntegerLiteral {{.*}}'int' 2
//DUMP: IntegerLiteral {{.*}}'int' 20
//DUMP: OMPIfClause
//DUMP: BinaryOperator{{.*}}'bool' '<'
//DUMP: ImplicitCastExpr{{.*}}'int' <LValueToRValue>
//DUMP: DeclRefExpr{{.*}}'int' lvalue Var{{.*}}'a' 'int'
//DUMP: ImplicitCastExpr{{.*}}'int' <LValueToRValue>
//DUMP: DeclRefExpr{{.*}}'int' lvalue Var{{.*}}'b' 'int'
//PRINT: #pragma ompx prefetch data(p:1:10, &b:2:20) if(a < b)

template <typename T, unsigned hint, unsigned size>
T run() {
  T foo[size];
  #pragma ompx prefetch data(&foo:hint:size)

  return foo[0];
}

int template_test() {
  double d;
  d = run<double,1,10>();
  return 0;
}

//DUMP: FunctionTemplateDecl {{.*}} <line:57:1, line:63:1> line:58:3
//DUMP: TemplateTypeParmDecl {{.*}} <line:57:11, col:20> col:20
//DUMP: NonTypeTemplateParmDecl {{.*}} <col:23, col:32> col:32 {{.*}} 'unsigned int' depth 0 index 1 hint
//DUMP: NonTypeTemplateParmDecl {{.*}} <col:38, col:47> col:47 {{.*}} 'unsigned int' depth 0 index 2 size
//DUMP: FunctionDecl {{.*}} <line:58:1, line:63:1> line:58:3 {{.*}} 'T ()'
//DUMP: CompoundStmt {{.*}} <col:9, line:63:1>
//DUMP: DeclStmt {{.*}} <line:59:3, col:14>
//DUMP: VarDecl {{.*}} <col:3, col:13> col:5 {{.*}} foo 'T[size]'
//DUMP: OMPPrefetchDirective {{.*}}openmp_standalone_directive
//DUMP: OMPDataClause
//DUMP: UnaryOperator {{.*}}'<dependent type>' prefix '&' cannot overflow
//DUMP: DeclRefExpr {{.*}}'T[size]' lvalue Var {{.*}} 'foo' 'T[size]'
//DUMP: DeclRefExpr {{.*}}'unsigned int' NonTypeTemplateParm {{.*}} 'hint' 'unsigned int'
//DUMP: DeclRefExpr {{.*}}'unsigned int' NonTypeTemplateParm {{.*}} 'size' 'unsigned int'
//DUMP: ReturnStmt {{.*}} <line:62:3, col:15>
//DUMP: ArraySubscriptExpr {{.*}} 'T' lvalue
//DUMP: DeclRefExpr {{.*}} <col:10> 'T[size]' lvalue Var {{.*}} 'foo' 'T[size]'
//DUMP: IntegerLiteral {{.*}} <col:14> 'int' 0
//DUMP: FunctionDecl {{.*}} <line:58:1, line:63:1> line:58:3 {{.*}} run 'double ()'
//DUMP: TemplateArgument type 'double'
//DUMP: BuiltinType {{.*}} 'double'
//DUMP: TemplateArgument integral 1
//DUMP: TemplateArgument integral 10
//DUMP: CompoundStmt {{.*}} <col:9, line:63:1>
//DUMP: DeclStmt {{.*}} <line:59:3, col:14>
//DUMP: VarDecl {{.*}} <col:3, col:13> col:5 {{.*}} foo 'double[10]'
//DUMP: OMPPrefetchDirective {{.*}}openmp_standalone_directive
//DUMP: OMPDataClause
//DUMP: UnaryOperator {{.*}}'double (*)[10]' prefix '&' cannot overflow
//DUMP: DeclRefExpr {{.*}}'double[10]' lvalue Var {{.*}} 'foo' 'double[10]'
//DUMP: SubstNonTypeTemplateParmExpr {{.*}}'unsigned int'
//DUMP: NonTypeTemplateParmDecl {{.*}} 'unsigned int' depth 0 index 1 hint
//DUMP: IntegerLiteral {{.*}}'unsigned int' 1
//DUMP: SubstNonTypeTemplateParmExpr {{.*}}'unsigned int'
//DUMP: NonTypeTemplateParmDecl {{.*}}'unsigned int' depth 0 index 2 size
//DUMP: IntegerLiteral {{.*}}'unsigned int' 10
//PRINT: #pragma ompx prefetch data(&foo:1U:10U)
#endif // HEADER
// end INTEL_COLLAB
