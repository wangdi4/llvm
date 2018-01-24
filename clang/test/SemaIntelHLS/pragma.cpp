//RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

//CHECK: FunctionDecl{{.*}}foo_unroll
void foo_unroll()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}Unroll Enable
  #pragma unroll
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}UnrollCount Numeric
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  #pragma unroll 4
  for (int i=0;i<32;++i) {}

  #pragma unroll // expected-error {{incompatible directives}}
  #pragma unroll 4
  for (int i=0;i<32;++i) {}

  #pragma unroll // expected-error {{duplicate directives}}
  #pragma unroll
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}foo_coalesce
void foo_coalesce()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}LoopCoalesce Enable
  #pragma loop_coalesce
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}LoopCoalesce Numeric
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  #pragma loop_coalesce 4
  for (int i=0;i<32;++i) {}

  #pragma loop_coalesce // expected-error {{incompatible directives}}
  #pragma loop_coalesce 4
  for (int i=0;i<32;++i) {}

  #pragma loop_coalesce // expected-error {{duplicate directives}}
  #pragma loop_coalesce
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}foo_ii
void foo_ii()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}II Numeric
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  #pragma ii 4
  for (int i=0;i<32;++i) {}

  #pragma ii // expected-warning {{expected value}}
  for (int i=0;i<32;++i) {}

  #pragma ii 4 // expected-error {{duplicate directives}}
  #pragma ii 8
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}foo_max_concurrency
void foo_max_concurrency()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}MaxConcurrency Numeric
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  #pragma max_concurrency 4
  for (int i=0;i<32;++i) {}

  #pragma max_concurrency // expected-warning {{expected value}}
  for (int i=0;i<32;++i) {}

  #pragma max_concurrency 4 // expected-error {{duplicate directives}}
  #pragma max_concurrency 8
  for (int i=0;i<32;++i) {}
}

struct IV_S {
  int arr1[10];
} ivs[20];

//CHECK: FunctionDecl{{.*}}ivdep1
void ivdep1(IV_S* sp)
{
  int i;
  //CHECK: LoopHintAttr{{.*}}IVDep LoopExpr
  //CHECK-NEXT: NULL
  //CHECK-NEXT: MemberExpr{{.*}}arr1
  //CHECK: DeclRefExpr{{.*}}'sp' 'struct IV_S *'
  #pragma ivdep array(sp->arr1)
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}tivdep 'void (struct IV_S *)'
//CHECK: LoopHintAttr{{.*}}IVDep LoopExpr
//CHECK-NEXT: NULL
//CHECK-NEXT: MemberExpr{{.*}}arr1
//CHECK: DeclRefExpr{{.*}}'tsp' 'struct IV_S *'
template <typename T>
void tivdep(T* tsp)
{
  int i;
  #pragma ivdep array(tsp->arr1)
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}t2ivdep 'void (int)'
//CHECK: LoopHintAttr{{.*}}IVDep LoopExpr
//CHECK-NEXT: NULL
//CHECK-NEXT: MemberExpr{{.*}}arr1
//CHECK: DeclRefExpr{{.*}}'lsp' 'struct IV_S *'
template <int item>
void t2ivdep(int i) {
  IV_S *lsp = &ivs[item];
  #pragma ivdep array(lsp->arr1)
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}foo_ivdep
void foo_ivdep()
{
  int myArray[40];
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}IVDep Enable
  #pragma ivdep
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}IVDep Numeric
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  #pragma ivdep safelen(4)
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}IVDep LoopExpr
  //CHECK-NEXT: NULL
  //CHECK-NEXT: DeclRefExpr{{.*}}myArray
  #pragma ivdep array(myArray)
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}IVDep Full
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  //CHECK-NEXT: DeclRefExpr{{.*}}myArray
  #pragma ivdep safelen(4) array(myArray)
  for (int i=0;i<32;++i) {}

  IV_S* p;
  tivdep(p);
  t2ivdep<4>(0);

  #pragma ivdep // expected-error {{duplicate directives}}
  #pragma ivdep
  for (int i=0;i<32;++i) {}

  #pragma ivdep safelen(4) safelen(8) // expected-warning {{'safelen' cannot appear multiple times}}
  for (int i=0;i<32;++i) {}

  #pragma ivdep array(myArray) array(myArray) // expected-warning {{'array' cannot appear multiple times}}
  for (int i=0;i<32;++i) {}

  #pragma ivdep array(typo_array) // expected-error {{use of undeclared identifier 'typo_array'}}
  for (int i=0;i<32;++i) {}

  IV_S *mysp = &ivs[5];
  #pragma ivdep array(mysp->lala) // expected-error {{no member named 'lala'}}
  for (int i=0;i<32;++i) {}
}
