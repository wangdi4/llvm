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

void foo1() {
  int start = 1;
  int length = 2;
  int stride = 4;
  #pragma omp target ompx_places(numa_domain,start:length:stride)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: ImplicitCastExpr {{.*}}'int' <LValueToRValue>
//DUMP: DeclRefExpr {{.*}}'int' lvalue OMPCapturedExpr {{.*}}'.capture_expr.' 'int'
//DUMP: ImplicitCastExpr {{.*}}'int' <LValueToRValue>
//DUMP: DeclRefExpr {{.*}}'int' lvalue OMPCapturedExpr {{.*}}'.capture_expr.' 'int'
//DUMP: ImplicitCastExpr {{.*}}'int' <LValueToRValue>
//DUMP: DeclRefExpr {{.*}}'int' lvalue OMPCapturedExpr {{.*}}'.capture_expr.' 'int'
//PRINT: #pragma omp target ompx_places(numa_domain,start:length:stride)

  #pragma omp target ompx_places(subnuma_domain,start:length)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: ImplicitCastExpr {{.*}}'int' <LValueToRValue>
//DUMP: DeclRefExpr {{.*}}'int' lvalue OMPCapturedExpr {{.*}}'.capture_expr.' 'int'
//DUMP: ImplicitCastExpr {{.*}}'int' <LValueToRValue>
//DUMP: DeclRefExpr {{.*}}'int' lvalue OMPCapturedExpr {{.*}}'.capture_expr.' 'int
//PRINT: #pragma omp target ompx_places(subnuma_domain,start:length)
  #pragma omp target ompx_places(numa_domain,start:length)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: ImplicitCastExpr {{.*}}'int' <LValueToRValue>
//DUMP: DeclRefExpr {{.*}}'int' lvalue OMPCapturedExpr {{.*}}'.capture_expr.' 'int'
//DUMP: ImplicitCastExpr {{.*}}'int' <LValueToRValue>
//DUMP: DeclRefExpr {{.*}}'int' lvalue OMPCapturedExpr {{.*}}'.capture_expr.' 'int'
//DUMP: <<<NULL>>>
//PRINT: #pragma omp target ompx_places(numa_domain,start:length)

  #pragma omp target ompx_places(subnuma_domain,start)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: ImplicitCastExpr {{.*}}'int' <LValueToRValue>
//DUMP: DeclRefExpr {{.*}}'int' lvalue OMPCapturedExpr {{.*}}'.capture_expr.' 'int'
//DUMP: <<<NULL>>>
//DUMP: <<<NULL>>>
//PRINT: #pragma omp target ompx_places(subnuma_domain,start)

  #pragma omp target ompx_places(start)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: ImplicitCastExpr {{.*}}'int' <LValueToRValue>
//DUMP: DeclRefExpr {{.*}}'int' lvalue OMPCapturedExpr {{.*}}'.capture_expr.' 'int'
//DUMP: <<<NULL>>>
//DUMP: <<<NULL>>>
//PRINT: #pragma omp target ompx_places(start)

  #pragma omp target ompx_places(0)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: IntegerLiteral {{.*}}'int' 0
//DUMP: <<<NULL>>>
//DUMP: <<<NULL>>>
//PRINT: #pragma omp target ompx_places(0)

  #pragma omp target ompx_places(subnuma_domain,0)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: IntegerLiteral {{.*}}'int' 0
//DUMP: <<<NULL>>>
//DUMP: <<<NULL>>>
//PRINT: #pragma omp target ompx_places(subnuma_domain,0)

  #pragma omp target ompx_places(0:2)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: IntegerLiteral {{.*}}'int' 0
//DUMP: IntegerLiteral {{.*}}'int' 2
//DUMP: <<<NULL>>>
//PRINT: #pragma omp target ompx_places(0:2)

  #pragma omp target ompx_places(start:2)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: DeclRefExpr {{.*}}'int' lvalue OMPCapturedExpr {{.*}}'.capture_expr.' 'int'
//DUMP: IntegerLiteral {{.*}}'int' 2
//DUMP: <<<NULL>>>
//PRINT: #pragma omp target ompx_places(start:2)

  #pragma omp target ompx_places(subnuma_domain,0:2)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: IntegerLiteral {{.*}}'int' 0
//DUMP: IntegerLiteral {{.*}}'int' 2
//DUMP: <<<NULL>>>
//PRINT: #pragma omp target ompx_places(subnuma_domain,0:2)

  #pragma omp target ompx_places(6:7:8)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: IntegerLiteral {{.*}}'int' 6
//DUMP: IntegerLiteral {{.*}}'int' 7
//DUMP: IntegerLiteral {{.*}}'int' 8
//PRINT: #pragma omp target ompx_places(6:7:8)

  #pragma omp target ompx_places(numa_domain,6:7:8)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: IntegerLiteral {{.*}}'int' 6
//DUMP: IntegerLiteral {{.*}}'int' 7
//DUMP: IntegerLiteral {{.*}}'int' 8
//PRINT: #pragma omp target ompx_places(numa_domain,6:7:8)

  #pragma omp target ompx_places(subnuma_domain,6:length:32)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: IntegerLiteral {{.*}}'int' 6
//DUMP: DeclRefExpr {{.*}}'int' lvalue OMPCapturedExpr {{.*}}'.capture_expr.' 'int'
//DUMP: IntegerLiteral {{.*}}'int' 32 
//PRINT: #pragma omp target ompx_places(subnuma_domain,6:length:32)

  #pragma omp target ompx_places(numa_domain,start:64:stride)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: DeclRefExpr {{.*}}'int' lvalue OMPCapturedExpr {{.*}}'.capture_expr.' 'int'
//DUMP: IntegerLiteral {{.*}}'int' 64
//DUMP: DeclRefExpr {{.*}}'int' lvalue OMPCapturedExpr {{.*}}'.capture_expr.' 'int'
//PRINT: #pragma omp target ompx_places(numa_domain,start:64:stride)
}

template <typename T, unsigned start, unsigned length, unsigned stride>
T run() {
//
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: DeclRefExpr {{.*}}'unsigned int' NonTypeTemplateParm {{.*}}'start' 'unsigned int'
//DUMP: DeclRefExpr {{.*}}'unsigned int' NonTypeTemplateParm {{.*}}'length' 'unsigned int'
//DUMP: DeclRefExpr {{.*}}'unsigned int' NonTypeTemplateParm {{.*}}'stride' 'unsigned int'
//PRINT: #pragma omp target ompx_places(subnuma_domain,start:length:stride)

  #pragma omp target ompx_places(subnuma_domain,start:length:stride)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: SubstNonTypeTemplateParmExpr {{.*}}'unsigned int'
//DUMP: NonTypeTemplateParmDecl {{.*}}referenced 'unsigned int' depth 0 index 1 start
//DUMP: IntegerLiteral {{.*}}'unsigned int' 1
//DUMP: SubstNonTypeTemplateParmExpr {{.*}}'unsigned int'
//DUMP: NonTypeTemplateParmDecl {{.*}}referenced 'unsigned int' depth 0 index 2 length
//DUMP: IntegerLiteral {{.*}}'unsigned int' 2
//DUMP: SubstNonTypeTemplateParmExpr {{.*}}'unsigned int'
//DUMP: NonTypeTemplateParmDecl {{.*}}referenced 'unsigned int' depth 0 index 3 stride
//PRINT: #pragma omp target ompx_places(subnuma_domain,1U:2U:3U)

  #pragma omp target ompx_places(subnuma_domain,start:length)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: SubstNonTypeTemplateParmExpr {{.*}}'unsigned int'
//DUMP: NonTypeTemplateParmDecl {{.*}}referenced 'unsigned int' depth 0 index 1 start
//DUMP: IntegerLiteral {{.*}}'unsigned int' 1
//DUMP: SubstNonTypeTemplateParmExpr {{.*}}'unsigned int'
//DUMP: NonTypeTemplateParmDecl {{.*}}referenced 'unsigned int' depth 0 index 2 length
//DUMP: IntegerLiteral {{.*}}'unsigned int' 2
//DUMP: <<<NULL>>>
//PRINT: #pragma omp target ompx_places(subnuma_domain,1U:2U)

  #pragma omp target ompx_places(subnuma_domain,start)
  {}
//DUMP: OMPTargetDirective {{.*}}
//DUMP: OMPOmpx_placesClause {{.*}}
//DUMP: SubstNonTypeTemplateParmExpr {{.*}}'unsigned int'
//DUMP: NonTypeTemplateParmDecl {{.*}}referenced 'unsigned int' depth 0 index 1 start
//DUMP: IntegerLiteral {{.*}}'unsigned int' 1
//DUMP: <<<NULL>>>
//DUMP: <<<NULL>>>
//PRINT: #pragma omp target ompx_places(subnuma_domain,1U)
  return;
}
int template_test() { run<void,1,2,3>(); return 0;
}
#endif // HEADER

// end INTEL_COLLAB
