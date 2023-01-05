// INTEL_COLLAB
// expected-no-diagnostics

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions \
//RUN:   -fcxx-exceptions -Wno-source-uses-openmp -Wno-openmp-clauses \
//RUN:   -ast-print %s | FileCheck %s --check-prefix=PRINT

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions \
//RUN:   -fcxx-exceptions -Wno-source-uses-openmp -Wno-openmp-clauses \
//RUN:   -ast-dump %s | FileCheck %s --check-prefix=DUMP

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions \
//RUN:   -fcxx-exceptions -Wno-source-uses-openmp -Wno-openmp-clauses \
//RUN:   -emit-pch -o %t %s

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions \
//RUN:   -fcxx-exceptions -Wno-source-uses-openmp -Wno-openmp-clauses \
//RUN:   -include-pch %t -ast-print %s | FileCheck %s --check-prefix=PRINT

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions \
//RUN:   -fcxx-exceptions -Wno-source-uses-openmp -Wno-openmp-clauses \
//RUN:   -include-pch %t -ast-dump-all %s | FileCheck %s --check-prefix=DUMP

#ifndef HEADER
#define HEADER

int test1() {
  short y;
  y = 2;

  #pragma omp target in_reduction(+ : y)
  { y++; }
  return y;
}

//DUMP: FunctionDecl {{.*}}
//DUMP: CompoundStmt {{.*}}
//DUMP: DeclStmt {{.*}}
//DUMP: VarDecl {{.*}} used y 'short'
//DUMP: BinaryOperator {{.*}} 'short' lvalue '='
//DUMP: DeclRefExpr {{.*}} 'short' lvalue Var {{.*}} 'y' 'short'
//DUMP: ImplicitCastExpr {{.*}} 'short' <IntegralCast>
//DUMP: IntegerLiteral {{.*}} 'int' 2
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPIn_reductionClause {{.*}}
//DUMP: DeclRefExpr {{.*}} 'short' lvalue Var {{.*}} 'y' 'short'
//PRINT: #pragma omp target in_reduction(+: y)

template <typename T, unsigned size>
T run() {
  T foo;
  #pragma omp target in_reduction(- : foo)
  { foo -= size; }
  return foo;
}

int template_test() {
  double d;
  d = run<double,1>();
  return 0;
}

//DUMP: FunctionTemplateDecl {{.*}}
//DUMP: TemplateTypeParmDecl {{.*}}
//DUMP: NonTypeTemplateParmDecl {{.*}} 'unsigned int' depth 0 index 1 size
//DUMP: FunctionDecl {{.*}} 'T ()'
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPIn_reductionClause {{.*}}
//DUMP: DeclRefExpr {{.*}} 'T' lvalue Var {{.*}} 'foo' 'T'
//DUMP: CompoundAssignOperator {{.*}} '<dependent type>' lvalue '-=' ComputeLHSTy='<dependent type>' ComputeResultTy='<dependent type>'
//DUMP: DeclRefExpr {{.*}} 'T' lvalue Var {{.*}} 'foo' 'T'
//DUMP: DeclRefExpr {{.*}} 'unsigned int' NonTypeTemplateParm {{.*}} 'size' 'unsigned int'

//DUMP: FunctionDecl {{.*}} run 'double ()'
//DUMP: TemplateArgument type 'double'
//DUMP: BuiltinType {{.*}} 'double'
//DUMP: TemplateArgument integral 1
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPIn_reductionClause {{.*}}
//DUMP: DeclRefExpr {{.*}} 'double':'double' lvalue Var {{.*}} 'foo' 'double':'double'
//DUMP: CompoundAssignOperator {{.*}} 'double':'double' lvalue '-=' ComputeLHSTy='double':'double' ComputeResultTy='double':'double'
//DUMP: DeclRefExpr {{.*}} 'double':'double' lvalue Var {{.*}} 'foo' 'double':'double'
//DUMP: ImplicitCastExpr {{.*}} 'double':'double' <IntegralToFloating>
//DUMP: SubstNonTypeTemplateParmExpr {{.*}} 'unsigned int'
//DUMP: NonTypeTemplateParmDecl {{.*}} referenced 'unsigned int' depth 0 index 1 size
//DUMP: IntegerLiteral {{.*}} 'unsigned int' 1

//DUMP: FunctionDecl {{.*}} template_test 'int ()'
//DUMP: CallExpr {{.*}} 'double':'double'
//DUMP: ImplicitCastExpr {{.*}} 'double (*)()' <FunctionToPointerDecay>
//DUMP: DeclRefExpr {{.*}} 'double ()' lvalue Function {{.*}} 'run' 'double ()' (FunctionTemplate {{.*}} 'run')
//PRINT: #pragma omp target in_reduction(-: foo)
//PRINT: #pragma omp target in_reduction(-: foo)
#endif // HEADER
// end INTEL_COLLAB
