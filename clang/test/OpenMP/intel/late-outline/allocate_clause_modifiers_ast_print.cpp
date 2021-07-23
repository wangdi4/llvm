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
//DUMP: FunctionDecl {{.*}} <line:45:1, line:67:1> line:45:5 {{.*}}foo 'int ()'
//DUMP: OMPParallelDirective {{.*}} <line:52:3, col:58>
//DUMP: OMPAllocateClause {{.*}} <col:24, col:40>
//DUMP: DeclRefExpr {{.*}} <col:33> 'int' lvalue Var {{.*}} 'a' 'int'
//DUMP: DeclRefExpr {{.*}} <col:35> 'short' lvalue Var {{.*}} 'b' 'short'
//DUMP: DeclRefExpr {{.*}} <col:37> 'char' lvalue Var {{.*}} 'c' 'char'
//DUMP: DeclRefExpr {{.*}} <col:39> 'float' lvalue Var {{.*}} 'd' 'float'
//DUMP: OMPParallelDirective {{.*}} <line:54:3, col:55>
//DUMP: OMPAllocateClause {{.*}} <col:24, col:43>
//DUMP: DeclRefExpr {{.*}} <col:42> 'int' lvalue Var {{.*}} 'a' 'int'
//DUMP: OMPParallelDirective {{.*}} <line:56:3, col:78>
//DUMP: OMPAllocateClause {{.*}} <col:24, col:60>
//DUMP: DeclRefExpr {{.*}} <col:53> 'int' lvalue Var {{.*}} 'a' 'int'
//DUMP: DeclRefExpr {{.*}} <col:55> 'short' lvalue Var {{.*}} 'b' 'short'
//DUMP: DeclRefExpr {{.*}} <col:57> 'char' lvalue Var {{.*}} 'c' 'char'
//DUMP: DeclRefExpr {{.*}} <col:59> 'float' lvalue Var {{.*}} 'd' 'float'
//DUMP: OMPParallelDirective {{.*}} <line:58:3, col:60>
//DUMP: OMPAllocateClause {{.*}} <col:24, col:46>
//DUMP: DeclRefExpr {{.*}} <col:43> 'int' lvalue Var {{.*}} 'a' 'int'
//DUMP: DeclRefExpr {{.*}} <col:45> 'short' lvalue Var {{.*}} 'b' 'short'
//DUMP: OMPParallelDirective {{.*}} <line:60:3, line:61:40>
//DUMP: OMPAllocateClause {{.*}} <line:60:24, col:70>
//DUMP: DeclRefExpr {{.*}} <col:63> 'int' lvalue Var {{.*}} 'a' 'int'
//DUMP: DeclRefExpr {{.*}} <col:65> 'short' lvalue Var {{.*}} 'b' 'short'
//DUMP: DeclRefExpr {{.*}} <col:67> 'char' lvalue Var {{.*}} 'c' 'char'
//DUMP: DeclRefExpr {{.*}} <col:69> 'float' lvalue Var {{.*}} 'd' 'float'
//DUMP: OMPParallelDirective {{.*}} <line:63:3, col:80>
//DUMP: OMPAllocateClause {{.*}} <col:24, col:66>
//DUMP: DeclRefExpr {{.*}} <col:63> 'short' lvalue Var {{.*}} 'b' 'short'
//DUMP: DeclRefExpr {{.*}} <col:65> 'int' lvalue Var {{.*}} 'a' 'int'
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

//DUMP: FunctionTemplateDecl {{.*}} <line:105:1, line:113:1> line:107:3 {{.*}}run
//DUMP: TemplateTypeParmDecl {{.*}} <line:105:11, col:20> col:20 {{.*}}referenced typename depth 0 index 0 T
//DUMP: NonTypeTemplateParmDecl {{.*}} <col:23, col:46> col:46 {{.*}}referenced 'omp_allocator_handle_t':'omp_allocator_handle_t' depth 0 index 1 MyAlloc
//DUMP: NonTypeTemplateParmDecl {{.*}} <line:106:11, col:20> col:20 {{.*}}referenced 'unsigned int' depth 0 index 2 size
//DUMP: NonTypeTemplateParmDecl {{.*}} <col:26, col:35> col:35 {{.*}}referenced 'unsigned int' depth 0 index 3 align
//DUMP: FunctionDecl {{.*}} <line:107:1, line:113:1> line:107:3 {{.*}}run 'T (T)'
//DUMP: ParmVarDecl {{.*}} <col:7, col:9> col:9 {{.*}}referenced param 'T'
//DUMP: CompoundStmt {{.*}} <col:16, line:113:1>
//DUMP: DeclStmt {{.*}} <line:108:3, col:14>
//DUMP: VarDecl {{.*}} <col:3, col:13> col:5 {{.*}}referenced foo 'T [size]'
//DUMP: OMPParallelDirective {{.*}} <line:109:3, line:110:69>
//DUMP: OMPAllocateClause {{.*}} <line:109:24, line:110:48>
//DUMP: DeclRefExpr {{.*}} <col:38> 'T [size]' lvalue Var {{.*}} 'foo' 'T [size]'
//DUMP: DeclRefExpr {{.*}} <col:43> 'T' lvalue ParmVar {{.*}} 'param' 'T'
//DUMP: OMPPrivateClause {{.*}} <col:50, col:68>
//DUMP: DeclRefExpr {{.*}} <col:58> 'T [size]' lvalue Var {{.*}} 'foo' 'T [size]'
//DUMP: DeclRefExpr {{.*}} <col:63> 'T' lvalue ParmVar {{.*}} 'param' 'T'
//PRINT: #pragma omp parallel allocate(allocator((omp_allocator_handle_t)2UL), align(2U): foo,param) private(foo,param)
#endif // HEADER
// end INTEL_COLLAB
