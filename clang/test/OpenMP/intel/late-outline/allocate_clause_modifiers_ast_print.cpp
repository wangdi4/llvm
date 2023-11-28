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

typedef enum omp_allocator_handle_t {
  omp_null_allocator = 0,
  omp_default_mem_alloc = 1,
  omp_large_cap_mem_alloc = 2,
  omp_const_mem_alloc = 3,
  omp_high_bw_mem_alloc = 4,
  omp_low_lat_mem_alloc = 5,
  omp_cgroup_mem_alloc = 6,
  omp_pteam_mem_alloc = 7,
  omp_thread_mem_alloc = 8,
  KMP_ALLOCATOR_MAX_HANDLE = __UINTPTR_MAX__
} omp_allocator_handle_t;

int foo() {
  int a;
  short b;
  char c;
  float d;
  omp_allocator_handle_t MyAlloc = omp_large_cap_mem_alloc;

  #pragma omp parallel allocate(a,b,c,d) private(a,b,c,d)
  {}
  #pragma omp parallel allocate(MyAlloc: a) private(a)
  {}
  #pragma omp parallel allocate(allocator(MyAlloc): a,b,c,d) private(a,b,c,d)
  {}
  #pragma omp parallel allocate(align(4): a,b) private(a,b)
  {}
  #pragma omp parallel allocate(allocator(MyAlloc), align(4): a,b,c,d) \
                       private(a,b,c,d)
  {}
  #pragma omp parallel allocate(align(2), allocator(MyAlloc): b,a) private(b,a)
  {}
  return a;

}
//DUMP: FunctionDecl {{.*}}foo 'int ()'
//DUMP: OMPParallelDirective
//DUMP: OMPAllocateClause
//DUMP: DeclRefExpr {{.*}}'a' 'int'
//DUMP: DeclRefExpr {{.*}}'b' 'short'
//DUMP: DeclRefExpr {{.*}}'c' 'char'
//DUMP: DeclRefExpr {{.*}}'d' 'float'
//DUMP: OMPParallelDirective
//DUMP: OMPAllocateClause
//DUMP: DeclRefExpr {{.*}}'a' 'int'
//DUMP: OMPParallelDirective
//DUMP: OMPAllocateClause
//DUMP: DeclRefExpr {{.*}}'a' 'int'
//DUMP: DeclRefExpr {{.*}}'b' 'short'
//DUMP: DeclRefExpr {{.*}}'c' 'char'
//DUMP: DeclRefExpr {{.*}}'d' 'float'
//DUMP: OMPParallelDirective
//DUMP: OMPAllocateClause
//DUMP: DeclRefExpr {{.*}}'a' 'int'
//DUMP: DeclRefExpr {{.*}}'b' 'short'
//DUMP: OMPParallelDirective
//DUMP: OMPAllocateClause
//DUMP: DeclRefExpr {{.*}}'a' 'int'
//DUMP: DeclRefExpr {{.*}}'b' 'short'
//DUMP: DeclRefExpr {{.*}}'c' 'char'
//DUMP: DeclRefExpr {{.*}}'d' 'float'
//DUMP: OMPParallelDirective
//DUMP: OMPAllocateClause
//DUMP: DeclRefExpr {{.*}}'b' 'short'
//DUMP: DeclRefExpr {{.*}}'a' 'int'
//PRINT: #pragma omp parallel allocate(a,b,c,d) private(a,b,c,d)
//PRINT: #pragma omp parallel allocate(MyAlloc: a) private(a)
//PRINT: #pragma omp parallel allocate(MyAlloc: a,b,c,d) private(a,b,c,d)
//PRINT: #pragma omp parallel allocate(align(4): a,b) private(a,b)
//PRINT: #pragma omp parallel allocate(allocator(MyAlloc), align(4): a,b,c,d) private(a,b,c,d)
//PRINT: #pragma omp parallel allocate(allocator(MyAlloc), align(2): b,a) private(b,a

template <typename T, omp_allocator_handle_t MyAlloc,
          unsigned size, unsigned align>
T run(T param) {
  T foo[size];
  #pragma omp parallel allocate(allocator(MyAlloc), \
                       align(align): foo, param) private(foo, param)
  {}
  return foo[0];
}

int template_test() {
  double d;
  d = run<double, omp_large_cap_mem_alloc, 10, 2>(d);
  return 0;
}

//DUMP: FunctionTemplateDecl {{.*}}run
//DUMP: TemplateTypeParmDecl {{.*}}typename depth 0 index 0 T
//DUMP: NonTypeTemplateParmDecl {{.*}}referenced 'omp_allocator_handle_t' depth 0 index 1 MyAlloc
//DUMP: NonTypeTemplateParmDecl {{.*}}referenced 'unsigned int' depth 0 index 2 size
//DUMP: NonTypeTemplateParmDecl {{.*}}referenced 'unsigned int' depth 0 index 3 align
//DUMP: FunctionDecl {{.*}}run 'T (T)'
//DUMP: ParmVarDecl {{.*}}referenced param 'T'
//DUMP: CompoundStmt
//DUMP: DeclStmt
//DUMP: VarDecl {{.*}}referenced foo 'T[size]'
//DUMP: OMPParallelDirective
//DUMP: OMPAllocateClause
//DUMP: DeclRefExpr {{.*}}'foo' 'T[size]'
//DUMP: DeclRefExpr {{.*}}'param' 'T'
//DUMP: OMPPrivateClause
//DUMP: DeclRefExpr {{.*}}'foo' 'T[size]'
//DUMP: DeclRefExpr {{.*}}'param' 'T'
//PRINT: #pragma omp parallel allocate(allocator((omp_allocator_handle_t)2UL), align(2U): foo,param) private(foo,param)
#endif // HEADER
// end INTEL_COLLAB
