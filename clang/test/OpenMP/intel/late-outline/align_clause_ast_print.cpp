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

int foo1() {
  char a;
  #pragma omp allocate(a) align(4) allocator(omp_pteam_mem_alloc)
  return a;

}
//DUMP: FunctionDecl {{.*}} <line:45:1, line:50:1> line:45:5 {{.*}}
//DUMP: DeclStmt {{.*}} <line:46:3, col:9>
//DUMP: VarDecl {{.*}} <col:3, col:8> col:8 {{.*}} a 'char'
//DUMP: OMPAllocateDeclAttr {{.*}} <line:47:24> {{.*}} OMPPTeamMemAlloc
//DUMP: DeclRefExpr {{.*}} <col:46> 'omp_allocator_handle_t' EnumConstant {{.*}} 'omp_pteam_mem_alloc' 'omp_allocator_handle_t'
//DUMP: ConstantExpr {{.*}} <col:33> 'int'
//DUMP: value: Int 4
//DUMP: IntegerLiteral {{.*}} <col:33> 'int' 4
//DUMP: DeclStmt {{.*}} <col:3, col:66>
//DUMP: OMPAllocateDecl {{.*}} <col:3> col:3
//DUMP: DeclRefExpr {{.*}} <col:24> 'char' lvalue Var {{.*}} 'a' 'char'
//DUMP: OMPAlignClause {{.*}} <col:27, col:34>
//DUMP: ConstantExpr {{.*}} <col:33> 'int'
//DUMP: value: Int 4
//DUMP: IntegerLiteral {{.*}} <col:33> 'int' 4
//DUMP: OMPAllocatorClause {{.*}} <col:36, col:65>
//DUMP: DeclRefExpr {{.*}} <col:46> 'omp_allocator_handle_t' EnumConstant {{.*}} 'omp_pteam_mem_alloc' 'omp_allocator_handle_t'
//PRINT: #pragma omp allocate(a) align(4) allocator(omp_pteam_mem_alloc)

int foo2() {
  char b;
  #pragma omp allocate(b) allocator(omp_low_lat_mem_alloc) align(2)
  return b;
}
//DUMP: FunctionDecl {{.*}} <line:70:1, line:74:1> line:70:5 {{.*}}
//DUMP: DeclStmt {{.*}} <line:71:3, col:9>
//DUMP: VarDecl {{.*}} <col:3, col:8> col:8 {{.*}} b 'char'
//DUMP: OMPAllocateDeclAttr {{.*}} <line:72:24> Implicit OMPLowLatMemAlloc
//DUMP: DeclRefExpr {{.*}} <col:37> 'omp_allocator_handle_t' EnumConstant {{.*}} 'omp_low_lat_mem_alloc' 'omp_allocator_handle_t'
//DUMP: ConstantExpr {{.*}} <col:66> 'int'
//DUMP: value: Int 2
//DUMP: IntegerLiteral {{.*}} <col:66> 'int' 2
//DUMP: DeclStmt {{.*}} <col:3, col:68>
//DUMP: OMPAllocateDecl {{.*}} <col:3> col:3
//DUMP: DeclRefExpr {{.*}} <col:24> 'char' lvalue Var {{.*}} 'b' 'char'
//DUMP: OMPAllocatorClause {{.*}} <col:27, col:58>
//DUMP: DeclRefExpr {{.*}} <col:37> 'omp_allocator_handle_t' EnumConstant {{.*}} 'omp_low_lat_mem_alloc' 'omp_allocator_handle_t'
//DUMP: OMPAlignClause {{.*}} <col:60, col:67>
//DUMP:  ConstantExpr {{.*}} <col:66> 'int'
//DUMP: value: Int 2
//DUMP: IntegerLiteral {{.*}} <col:66> 'int' 2
//PRINT: #pragma omp allocate(b) allocator(omp_low_lat_mem_alloc) align(2)

template <typename T, unsigned size>
T run() {
  T foo;
  #pragma omp allocate(foo) align(size)
  return size;
}

int template_test() {
  double d;
  d = run<double,1>();
  return 0;
}

//DUMP: FunctionTemplateDecl {{.*}} <line:94:1, line:99:1> line:95:3
//DUMP: TemplateTypeParmDecl {{.*}} <line:94:11, col:20> col:20
//DUMP: NonTypeTemplateParmDecl {{.*}} <col:23, col:32> col:32 {{.*}} 'unsigned int' depth 0 index 1 size
//DUMP: FunctionDecl {{.*}} <line:95:1, line:99:1> line:95:3 {{.*}} 'T ()'
//DUMP: DeclStmt {{.*}} <line:97:3, col:40>
//DUMP: OMPAllocateDecl {{.*}} <col:3> col:3
//DUMP: DeclRefExpr {{.*}} <col:24> 'T' lvalue Var {{.*}} 'foo' 'T'
//DUMP: OMPAlignClause {{.*}} <col:29, col:39>
//DUMP: DeclRefExpr {{.*}} <col:35> 'unsigned int' NonTypeTemplateParm {{.*}} 'size' 'unsigned int'
//DUMP: FunctionDecl {{.*}} <line:95:1, line:99:1> line:95:3 {{.*}} run 'double ()'
//DUMP: TemplateArgument type 'double'
//DUMP: BuiltinType {{.*}} 'double'
//DUMP: TemplateArgument integral 1
//DUMP: OMPAllocateDeclAttr {{.*}} <line:97:24> Implicit OMPNullMemAlloc
//DUMP: ConstantExpr {{.*}} <col:35> 'unsigned int'
//DUMP: value: Int 1
//DUMP: SubstNonTypeTemplateParmExpr {{.*}} <col:35> 'unsigned int'
//DUMP: NonTypeTemplateParmDecl {{.*}} <line:94:23, col:32> col:32 {{.*}} 'unsigned int' depth 0 index 1 size
//DUMP: IntegerLiteral {{.*}} <line:97:35> 'unsigned int' 1
//DUMP: OMPAllocateDecl {{.*}} <col:3> col:3
//DUMP: DeclRefExpr {{.*}} <col:24> 'double' lvalue Var {{.*}} 'foo' 'double'
//DUMP: OMPAlignClause {{.*}} <col:29, col:39>
//DUMP: ConstantExpr {{.*}} <col:35> 'unsigned int'
//DUMP: value: Int 1
//DUMP: SubstNonTypeTemplateParmExpr {{.*}} <col:35> 'unsigned int'
//DUMP: NonTypeTemplateParmDecl {{.*}} <line:94:23, col:32> col:32 {{.*}} 'unsigned int' depth 0 index 1 size
//DUMP: IntegerLiteral {{.*}} <line:97:35> 'unsigned int' 1
//PRINT: #pragma omp allocate(foo) align(size)
//PRINT: #pragma omp allocate(foo) align(1U)
#endif // HEADER
// end INTEL_COLLAB
