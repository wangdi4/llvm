// RUN: %clang_cc1 -verify -fopenmp -ast-dump %s | FileCheck %s
// expected-no-diagnostics

int usedVar;
int otherVar;

void bar1() {}
void bar2() {}
void bar3() {}
#pragma omp declare target
void foo();
#pragma omp end declare target

void foo()
{
  #pragma omp target
  {
   bar1(); usedVar++;
  }
  bar2();
}

void foo1()
{
  bar3();
  otherVar++;
}

//CHECK: FunctionDecl{{.*}}bar1
//CHECK-NEXT: CompoundStmt
//CHECK-NEXT: OMPDeclareTargetDeclAttr{{.*}}Implicit
//CHECK: FunctionDecl{{.*}}bar2
//CHECK-NEXT: CompoundStmt
//CHECK-NEXT: OMPDeclareTargetDeclAttr{{.*}}Implicit
//CHECK: FunctionDecl{{.*}}bar3
//CHECK-NEXT: CompoundStmt
//CHECK-NEXT: FunctionDecl{{.*}}foo
//CHECK-NEXT: OMPDeclareTargetDeclAttr{{.*}}Implicit
//CHECK-NEXT: FunctionDecl{{.*}}foo
//CHECK: OMPDeclareTargetDeclAttr{{.*}}Implicit
//CHECK: FunctionDecl{{.*}}foo1
//CHECK-NOT: OMPDeclareTargetDeclAttr
