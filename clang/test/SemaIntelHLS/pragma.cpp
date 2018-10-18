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

  #pragma unroll
  #pragma unroll 4 // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma unroll
  #pragma unroll // expected-error {{duplicate directives}}
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

  #pragma loop_coalesce
  #pragma loop_coalesce 4 // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma loop_coalesce
  #pragma loop_coalesce // expected-error {{duplicate directives}}
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

  #pragma ii 4
  #pragma ii 8 // expected-error {{duplicate directives}}
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

  #pragma max_concurrency 4
  #pragma max_concurrency 8 // expected-error {{duplicate directives}}
  for (int i=0;i<32;++i) {}
}

struct IV_S {
  int arr1[10];
} ivs[20];

//CHECK: FunctionDecl{{.*}}ivdep1
void ivdep1(IV_S* sp)
{
  int i;
  //CHECK: LoopHintAttr{{.*}}IVDepHLS LoopExpr
  //CHECK-NEXT: NULL
  //CHECK-NEXT: MemberExpr{{.*}}arr1
  //CHECK: DeclRefExpr{{.*}}'sp' 'IV_S *'
  #pragma ivdep array(sp->arr1)
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}tivdep 'void (IV_S *)'
//CHECK: LoopHintAttr{{.*}}IVDepHLS LoopExpr
//CHECK-NEXT: NULL
//CHECK-NEXT: MemberExpr{{.*}}arr1
//CHECK: DeclRefExpr{{.*}}'tsp' 'IV_S *'
template <typename T>
void tivdep(T* tsp)
{
  int i;
  #pragma ivdep array(tsp->arr1)
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}t2ivdep 'void (int)'
//CHECK: LoopHintAttr{{.*}}IVDepHLS LoopExpr
//CHECK-NEXT: NULL
//CHECK-NEXT: MemberExpr{{.*}}arr1
//CHECK: DeclRefExpr{{.*}}'lsp' 'IV_S *'
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
  //CHECK-NEXT: LoopHintAttr{{.*}}IVDepHLS Enable
  #pragma ivdep
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}IVDepHLS Numeric
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  #pragma ivdep safelen(4)
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}IVDepHLS LoopExpr
  //CHECK-NEXT: NULL
  //CHECK-NEXT: DeclRefExpr{{.*}}myArray
  #pragma ivdep array(myArray)
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}IVDepHLS Full
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  //CHECK-NEXT: DeclRefExpr{{.*}}myArray
  #pragma ivdep safelen(4) array(myArray)
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}IVDepHLS Full
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  //CHECK-NEXT: DeclRefExpr{{.*}}myArray
  //CHECK-NEXT: LoopHintAttr{{.*}}IVDepHLS LoopExpr
  //CHECK-NEXT: NULL
  //CHECK-NEXT: DeclRefExpr{{.*}}dArray
  double dArray[42];
  #pragma ivdep safelen(4) array(myArray)
  #pragma ivdep array(dArray)
  for (int i=0;i<32;++i) {}

  IV_S* p;
  tivdep(p);
  t2ivdep<4>(0);

  //okay now
  #pragma ivdep safelen(4)
  #pragma ivdep safelen(8) array(myArray)
  for (int i=0;i<32;++i) {}

  #pragma ivdep
  #pragma ivdep // expected-error {{duplicate directives}}
  for (int i=0;i<32;++i) {}

  //expected-warning@+1{{'safelen' cannot appear multiple times}}
  #pragma ivdep safelen(4) safelen(8)
  for (int i=0;i<32;++i) {}

  //expected-warning@+1{{'array' cannot appear multiple times}}
  #pragma ivdep array(myArray) array(myArray)
  for (int i=0;i<32;++i) {}

  //expected-error@+1{{use of undeclared identifier 'typo_array'}}
  #pragma ivdep array(typo_array)
  for (int i=0;i<32;++i) {}

  IV_S *mysp = &ivs[5];
  //expected-error@+1{{no member named 'lala'}}
  #pragma ivdep array(mysp->lala)
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}foo_ii_at_most
void foo_ii_at_most()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}IIAtMost Numeric
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  #pragma ii_at_most 4
  for (int i=0;i<32;++i) {}

  #pragma ii_at_most // expected-warning {{expected value}}
  for (int i=0;i<32;++i) {}

  #pragma ii_at_most 4
  #pragma ii_at_most 8 // expected-error {{duplicate directives}}
  for (int i=0;i<32;++i) {}
  int v;
  #pragma ii_at_most 4
    v = 0; // expected-error {{expected a for, while, or do-while loop to follow}}

}

//CHECK: FunctionDecl{{.*}}foo_ii_at_least
void foo_ii_at_least()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}IIAtLeast Numeric
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  #pragma ii_at_least 4
  for (int i=0;i<32;++i) {}

  #pragma ii_at_least // expected-warning {{expected value}}
  for (int i=0;i<32;++i) {}

  #pragma ii_at_least 4
  #pragma ii_at_least 8 // expected-error {{duplicate directives}}
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}foo_speculated_iterations
void foo_speculated_iterations()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}SpeculatedIterations Numeric
  //CHECK-NEXT: IntegerLiteral{{.*}} 4
  #pragma speculated_iterations 4
  for (int i=0;i<32;++i) {}

  #pragma speculated_iterations // expected-warning {{expected value}}
  for (int i=0;i<32;++i) {}

  #pragma speculated_iterations 4
  #pragma speculated_iterations 8 // expected-error {{duplicate directives}}
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}foo_min_ii_at_target_fmax
void foo_min_ii_at_target_fmax()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}MinIIAtFmax Enable
  #pragma min_ii_at_target_fmax
  for (int i=0;i<32;++i) {}

  #pragma min_ii_at_target_fmax
  #pragma min_ii_at_target_fmax  // expected-error {{duplicate directives}}
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}foo_ii_most_least_fmax
void foo_ii_most_least_fmax()
{
  #pragma ii_at_least 1
  #pragma ii_at_most  1  // expected-error {{duplicate directives}}
  #pragma ii  1  // expected-error {{duplicate directives}}
  #pragma min_ii_at_target_fmax  // expected-error {{duplicate directives}}
  for (int i=0;i<32;++i) {}

}

//CHECK: FunctionDecl{{.*}}foo_disable_loop_pipelining
void foo_disable_loop_pipelining()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}DisableLoopPipelining Enable
  #pragma disable_loop_pipelining
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining
  #pragma disable_loop_pipelining  // expected-error {{duplicate directives}}
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining
  #pragma max_concurrency 100  // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma max_concurrency 100  // expected-error {{incompatible directives}}
  #pragma disable_loop_pipelining
  for (int i=0;i<32;++i) {}

  #pragma ii  1 // expected-error {{incompatible directives}}
  #pragma max_concurrency 100
  #pragma disable_loop_pipelining
  for (int i=0;i<32;++i) {}

  int i, myArray[10];
  #pragma disable_loop_pipelining
  #pragma ivdep array(myArray) // expected-error {{incompatible directives}}
  #pragma ivdep safelen(8)  // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma ivdep safelen(8)  // expected-error {{incompatible directives}}
  #pragma disable_loop_pipelining
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining
  #pragma ii 4  // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining
  #pragma ii_at_most 4 // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma ii_at_most 4 // expected-error {{incompatible directives}}
  #pragma disable_loop_pipelining
  for (int i=0;i<32;++i) {}

  #pragma ii_at_least 4444 // expected-error {{incompatible directives}}
  #pragma disable_loop_pipelining
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining
  #pragma ii_at_least 4 // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining
  #pragma min_ii_at_target_fmax  // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma min_ii_at_target_fmax  // eaxpected-error {{incompatible directives}}
  #pragma disable_loop_pipelining
  for (int i=0;i<32;++i) {}

  #pragma speculated_iterations 4  // expected-error {{incompatible directives}}
  #pragma disable_loop_pipelining
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining
  #pragma speculated_iterations 4  // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}nontypeargument
template <int size>
void nontypeargument()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}IIAtLeast Numeric
  //CHECK-NEXT-NEXT: IntegerLiteral{{.*}}100
  #pragma ii_at_least  size
  for (int i=0;i<32;++i) {}
}

int main()
{
  nontypeargument<100>();
}
