// INTEL_COLLAB
//
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -ast-print %s \
// RUN:  | FileCheck %s --check-prefix=PRINT

// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -ast-dump %s \
// RUN:  | FileCheck %s --check-prefix=DUMP

// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -emit-pch -o %t.pch %s

// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -ast-print \
// RUN: -include-pch %t.pch %s | FileCheck %s --check-prefix=PRINT

// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -ast-dump-all \
// RUN:  -include-pch %t.pch %s | FileCheck %s --check-prefix=DUMP

// expected-no-diagnostics

#ifndef HEADER
#define HEADER

//PRINT: #pragma omp declare target
//PRINT: int foo();
//PRINT: #pragma omp end declare target
//DUMP: FunctionDecl{{.*}}foo 'int ()'
//DUMP-NEXT: OMPDeclareTargetDeclAttr{{.*}}Implicit MT_To DT_Any
#pragma ompx declare target function
int foo();

//PRINT: #pragma omp declare target
//PRINT: int foo1();
//PRINT: #pragma omp end declare target
//DUMP: FunctionDecl{{.*}}foo1 'int ()'
//DUMP-NEXT: OMPDeclareTargetDeclAttr{{.*}}Implicit MT_To DT_Any
[[ompx::directive(declare target function)]]
int foo1();

//PRINT: #pragma omp declare simd
//PRINT: #pragma omp declare target
//PRINT: int foo2();
//PRINT: #pragma omp end declare target
//DUMP: FunctionDecl{{.*}}foo2 'int ()'
//DUMP-NEXT: OMPDeclareSimdDeclAttr
//DUMP: OMPDeclareTargetDeclAttr{{.*}}Implicit MT_To DT_Any
[[ompx::directive(declare target function)]]
[[omp::directive(declare simd)]]
int foo2();

//PRINT: #pragma omp declare target
//PRINT: #pragma omp declare simd
//PRINT: int foo3();
//PRINT: #pragma omp end declare target
//DUMP: FunctionDecl{{.*}}foo3 'int ()'
//DUMP-NEXT: OMPDeclareTargetDeclAttr{{.*}}Implicit MT_To DT_Any
//DUMP: OMPDeclareSimdDeclAttr
[[omp::directive(declare simd)]]
[[ompx::directive(declare target function)]]
int foo3();

//PRINT: #pragma omp declare target
//PRINT: #pragma omp declare simd
//PRINT: int foo4();
//PRINT: #pragma omp end declare target
//DUMP: FunctionDecl{{.*}}foo4 'int ()'
//DUMP-NEXT: OMPDeclareTargetDeclAttr{{.*}}Implicit MT_To DT_Any
//DUMP: OMPDeclareSimdDeclAttr
[[omp::sequence(directive(declare simd),
                ompx::directive(declare target function))]]
int foo4();

//PRINT: #pragma omp declare simd
//PRINT: #pragma omp declare target
//PRINT: int foo5();
//PRINT: #pragma omp end declare target
//DUMP: FunctionDecl{{.*}}foo5 'int ()'
//DUMP-NEXT: OMPDeclareSimdDeclAttr
//DUMP: OMPDeclareTargetDeclAttr{{.*}}Implicit MT_To DT_Any
[[omp::sequence(ompx::directive(declare target function),
                directive(declare simd))]]
int foo5();

//PRINT: #pragma omp declare target
//PRINT: #pragma omp declare simd
//PRINT: int foo6();
//PRINT: #pragma omp end declare target
//DUMP: FunctionDecl{{.*}}foo6 'int ()'
//DUMP-NEXT: OMPDeclareTargetDeclAttr{{.*}}Implicit MT_To DT_Any
//DUMP: OMPDeclareSimdDeclAttr
[[omp::sequence(omp::directive(declare simd),
                ompx::directive(declare target function))]]
int foo6();

//PRINT: #pragma omp declare simd
//PRINT: #pragma omp declare target
//PRINT: int foo7();
//PRINT: #pragma omp end declare target
//DUMP: FunctionDecl{{.*}}foo7 'int ()'
//DUMP-NEXT: OMPDeclareSimdDeclAttr
//DUMP: OMPDeclareTargetDeclAttr{{.*}}Implicit MT_To DT_Any
[[omp::sequence(ompx::directive(declare target function),
                omp::directive(declare simd))]]
int foo7();

//PRINT: #pragma omp declare target device_type(host)
//PRINT: int foo8();
//PRINT: #pragma omp end declare target
//DUMP: FunctionDecl{{.*}}foo8 'int ()'
//DUMP-NEXT: OMPDeclareTargetDeclAttr{{.*}}Implicit MT_To DT_Host
#pragma ompx declare target function device_type(host)
int foo8();

//PRINT: #pragma omp declare target device_type(nohost)
//PRINT: int foo9();
//PRINT: #pragma omp end declare target
//DUMP: FunctionDecl{{.*}}foo9 'int ()'
//DUMP-NEXT: OMPDeclareTargetDeclAttr{{.*}}Implicit MT_To DT_NoHost
#pragma ompx declare target function device_type(nohost)
int foo9();

//PRINT: #pragma omp declare target
//PRINT: int foo10();
//PRINT: #pragma omp end declare target
//DUMP: FunctionDecl{{.*}}foo10 'int ()'
//DUMP-NEXT: OMPDeclareTargetDeclAttr{{.*}}Implicit MT_To DT_Any
#pragma ompx declare target function device_type(any)
int foo10();

//PRINT: #pragma omp declare target device_type(host)
//PRINT: int foo11();
//PRINT: #pragma omp end declare target
//DUMP: FunctionDecl{{.*}}foo11 'int ()'
//DUMP-NEXT: OMPDeclareTargetDeclAttr{{.*}}Implicit MT_To DT_Host
[[ompx::directive(declare target function device_type(host))]]
int foo11();

//PRINT: #pragma omp declare target device_type(nohost)
//PRINT: int foo12();
//PRINT: #pragma omp end declare target
//DUMP: FunctionDecl{{.*}}foo12 'int ()'
//DUMP-NEXT: OMPDeclareTargetDeclAttr{{.*}}Implicit MT_To DT_NoHost
[[ompx::directive(declare target function device_type(nohost))]]
int foo12();

//PRINT: #pragma omp declare target
//PRINT: int foo13();
//PRINT: #pragma omp end declare target
//DUMP: FunctionDecl{{.*}}foo13 'int ()'
//DUMP-NEXT: OMPDeclareTargetDeclAttr{{.*}}Implicit MT_To DT_Any
[[ompx::directive(declare target function device_type(any))]]
int foo13();

struct A {
  int i;
  //PRINT: #pragma omp declare target
  //PRINT: int bar();
  //PRINT: #pragma omp end declare target
  //DUMP: CXXMethodDecl{{.*}}bar 'int (){{.*}}'
  //DUMP-NEXT: OMPDeclareTargetDeclAttr{{.*}}Implicit MT_To DT_Any
  #pragma ompx declare target function
  int bar();
};

#endif // HEADER
// end INTEL_COLLAB
