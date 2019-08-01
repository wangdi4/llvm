//RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s
//RUN: %clang_cc1 -fhls -fsyntax-only -fintel-compatibility -ast-dump -verify -pedantic %s | FileCheck %s

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

//CHECK: FunctionDecl{{.*}}foo_max_interleaving
void foo_max_interleaving()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}MaxInterleaving Numeric
  //CHECK-NEXT: IntegerLiteral{{.*}}1
  #pragma max_interleaving 1
  for (int i=0;i<32;++i) {}
  #pragma max_interleaving // expected-warning {{expected value}}
  for (int i=0;i<32;++i) {}
  #pragma max_interleaving 4
  #pragma max_interleaving 8 // expected-error {{duplicate directives}}
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
  //CHECK-NEXT: LoopHintAttr{{.*}}IVDepHLS{{.*}}Enable
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

  // Tests for redundant ivdep pragma directives
  // 1. Fully overlapping pragmas [optionally with safelens]:
  // #pragma ivdep [safelen(n)]
  // #pragma ivdep [safelen(m)]
  // In this case, the max(n,m) will be used by the optimizer. Thus the
  // smaller one is redundant and should trigger a warning. No "safelen"
  // is equivalent to safelen(infinity).

  #pragma ivdep safelen(4) // expected-warning {{redundant directive}}
  #pragma ivdep safelen(8) // expected-note {{overriding ivdep pragma is specified here}}
  for (int i=0;i<32;++i) {}

  #pragma ivdep safelen(4) // expected-note {{overriding ivdep pragma is specified here}}
  #pragma ivdep safelen(2) // expected-warning {{redundant directive}}
  for (int i=0;i<32;++i) {}

  #pragma ivdep safelen(4)
  #pragma ivdep safelen(4) // expected-error {{duplicate directive}}
  for (int i=0;i<32;++i) {}

  #pragma ivdep safelen(8) // expected-warning {{redundant directive}}
  #pragma ivdep            // expected-note {{overriding ivdep pragma is specified here}}
  for (int i=0;i<32;++i) {}

  #pragma ivdep // expected-note {{overriding ivdep pragma is specified here}}
  #pragma ivdep safelen(8) // expected-warning {{redundant directive}}
  for (int i=0;i<32;++i) {}

  // 2. Partially overlapping pragmas:
  // #pragma ivdep [safelen(n)]
  // #pragma ivdep array(a) [safelen(m)]
  // In this case, the max(n,m) will be used for array a by the optimizer.
  // Thus if m <= n, the "array" pragma is redundant and should trigger a
  // warning.

  //expected-warning@+3 {{redundant directive}}
  //expected-note@+1 {{overriding ivdep pragma is specified here}}
  #pragma ivdep
  #pragma ivdep array(myArray) safelen(4)
  for (int i=0;i<32;++i) {}

  //expected-warning@+2 {{redundant directive}}
  //expected-note@+2 {{overriding ivdep pragma is specified here}}
  #pragma ivdep array(myArray) safelen(4)
  #pragma ivdep
  for (int i=0;i<32;++i) {}

  //expected-warning@+3 {{redundant directive}}
  //expected-note@+1 {{overriding ivdep pragma is specified here}}
  #pragma ivdep safelen(8)
  #pragma ivdep array(myArray) safelen(4)
  for (int i=0;i<32;++i) {}

  //expected-warning@+2 {{redundant directive}}
  //expected-note@+2 {{overriding ivdep pragma is specified here}}
  #pragma ivdep array(myArray) safelen(4)
  #pragma ivdep safelen(8)
  for (int i=0;i<32;++i) {}

  //expected-warning@+2 {{redundant directive}}
  //expected-note@+2 {{overriding ivdep pragma is specified here}}
  #pragma ivdep array(myArray) safelen(8)
  #pragma ivdep safelen(8)
  for (int i=0;i<32;++i) {}

  //expected-warning@+3 {{redundant directive}}
  //expected-note@+1 {{overriding ivdep pragma is specified here}}
  #pragma ivdep safelen(8)
  #pragma ivdep array(myArray) safelen(8)
  for (int i=0;i<32;++i) {}

  #pragma ivdep safelen(6) array(myArray)
  #pragma ivdep safelen(4) // not redundant directive since  m > n
  for (int i=0;i<32;++i) {}

  #pragma ivdep safelen(4) // not redundant directive since  m > n
  #pragma ivdep safelen(6) array(myArray)
  for (int i=0;i<32;++i) {}

  int myArray2[24];
  //expected-warning@+1 {{redundant directive}}
  #pragma ivdep array(myArray) safelen(4)
  #pragma ivdep array(myArray2) safelen(16)  // this one not redundant
  #pragma ivdep safelen(8)
  //expected-note@-1 {{overriding ivdep pragma is specified here}}
  for (int i=0;i<32;++i) {}

  //expected-warning@+3 {{redundant directive}}
  //expected-note@+1 {{overriding ivdep pragma is specified here}}
  #pragma ivdep
  #pragma ivdep array(myArray)
  for (int i=0;i<32;++i) {}

  //expected-warning@+2 {{redundant directive}}
  //expected-note@+2 {{overriding ivdep pragma is specified here}}
  #pragma ivdep array(myArray)
  #pragma ivdep
  for (int i=0;i<32;++i) {}

  //expected-warning@+6{{redundant directive}}
  //expected-note@+6 {{overriding ivdep pragma is specified here}}
  //expected-warning@+6 {{redundant directive}}
  //expected-note@+4 {{overriding ivdep pragma is specified here}}
  //expected-warning@+5 {{redundant directive}}
  //expected-note@+2 {{overriding ivdep pragma is specified here}}
  #pragma ivdep array(myArray)
  #pragma ivdep
  #pragma ivdep safelen(8)
  #pragma ivdep array(myArray) safelen(16)
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

  #pragma speculated_iterations 0
  for (int i=0;i<32;++i) {}

  #pragma speculated_iterations -1 // expected-error {{must be non-negative}}
  for (int i=0;i<32;++i) {}

  #pragma speculated_iterations 2147483648 // expected-error {{is too large}}
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

  #pragma max_concurrency 100
  #pragma disable_loop_pipelining // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma ii  1
  #pragma max_concurrency 100
  #pragma disable_loop_pipelining // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  int i, myArray[10];
  #pragma disable_loop_pipelining
  #pragma ivdep array(myArray) // expected-error {{incompatible directives}}
  #pragma ivdep safelen(8)  // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma ivdep safelen(8)
  #pragma disable_loop_pipelining // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining
  #pragma ii 4  // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining
  #pragma ii_at_most 4 // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma ii_at_most 4
  #pragma disable_loop_pipelining // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma ii_at_least 4444
  #pragma disable_loop_pipelining // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining
  #pragma ii_at_least 4 // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining
  #pragma min_ii_at_target_fmax  // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma min_ii_at_target_fmax
  #pragma disable_loop_pipelining // eaxpected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma speculated_iterations 4
  #pragma disable_loop_pipelining // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining
  #pragma speculated_iterations 4  // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}foo_force_hyperopt
void foo_force_hyperopt()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}ForceHyperopt Enable
  #pragma force_hyperopt
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: LoopHintAttr{{.*}}ForceHyperopt Disable
  #pragma force_no_hyperopt
  for (int i=0;i<32;++i) {}

  #pragma force_hyperopt
  #pragma force_hyperopt // expected-error {{duplicate directives}}
  for (int i=0;i<32;++i) {}

  #pragma force_hyperopt
  #pragma force_no_hyperopt // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma force_no_hyperopt
  #pragma force_hyperopt // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma force_no_hyperopt
  #pragma force_no_hyperopt // expected-error {{duplicate directives}}
  for (int i=0;i<32;++i) {}

  #pragma force_hyperopt
  #pragma disable_loop_pipelining // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma force_no_hyperopt
  #pragma disable_loop_pipelining // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining
  #pragma force_hyperopt // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining
  #pragma force_no_hyperopt // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  int v;
  #pragma force_hyperopt
   v = 0; // expected-error {{expected a for, while, or do-while loop to follow}}
  #pragma force_no_hyperopt
   v = 0; // expected-error {{expected a for, while, or do-while loop to follow}}

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

template <int size>
void nontypeargument1()
{
  #pragma speculated_iterations  size //expected-error {{must be non-negative}}
  for (int i=0;i<32;++i) {}
}

template <int size>
void nontypeargument2()
{
  #pragma speculated_iterations size
  for (int i=0;i<32;++i) {}
}

int main()
{
  nontypeargument<100>();
  nontypeargument2<0>();
  nontypeargument1<-1>(); //expected-note {{requested here}}
}
