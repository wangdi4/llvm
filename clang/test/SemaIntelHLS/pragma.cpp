//RUN: %clang_cc1 -fhls -fsyntax-only -verify -DTRIGGER_ERROR %s
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
  //CHECK-NEXT: SYCLIntelFPGALoopCoalesceAttr
  #pragma loop_coalesce
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelFPGALoopCoalesceAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  #pragma loop_coalesce 4
  for (int i=0;i<32;++i) {}

  #pragma loop_coalesce // expected-error {{duplicate Intel FPGA loop attribute}}
  #pragma loop_coalesce 4
  for (int i=0;i<32;++i) {}

  #pragma loop_coalesce // expected-error {{duplicate Intel FPGA loop attribute}}
  #pragma loop_coalesce
  for (int i=0;i<32;++i) {}

  // expected-warning@+1 {{extra tokens at end of '#pragma loop_coalesce' - ignored}}
  #pragma loop_coalesce (2) int c[10];

  // expected-warning@+1 {{extra tokens at end of '#pragma loop_coalesce' - ignored}}
  #pragma loop_coalesce (2) (4)
  for (int i=0;i<32;++i) {}

  // expected-warning@+1 {{extra tokens at end of '#pragma ' - ignored}}
  #pragma loop_coalesce (2,4)
  for (int i=0;i<32;++i) {}

    // expected-error@+1 {{invalid value '0'; must be positive}}
  #pragma loop_coalesce 0
  for (int i=0;i<32;++i) {}

  // expected-error@+1 {{invalid value '-1'; must be positive}}
  #pragma loop_coalesce -1
  for (int i=0;i<32;++i) {}

  // expected-error@+1 {{invalid argument of type 'const char [8]'; expected an integer type}}
  #pragma loop_coalesce "test123"
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}foo_ii
void foo_ii()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelFPGAIIAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  #pragma ii 4
  for (int i=0;i<32;++i) {}

  #pragma ii // expected-warning {{expected value}}
  for (int i=0;i<32;++i) {}

  #pragma ii 4 // expected-error {{duplicate Intel FPGA loop attribute}}
  #pragma ii 8
  for (int i=0;i<32;++i) {}

  // expected-warning@+1 {{extra tokens at end of '#pragma ii' - ignored}}
  #pragma ii (2) int c[10];

  // expected-warning@+1 {{extra tokens at end of '#pragma ii' - ignored}}
  #pragma ii (2) (4)
  for (int i=0;i<32;++i) {}

  // expected-warning@+1 {{extra tokens at end of '#pragma ' - ignored}}
  #pragma ii (2,4)
  for (int i=0;i<32;++i) {}

  // expected-error@+1 {{invalid value '0'; must be positive}}
  #pragma ii 0
  for (int i=0;i<32;++i) {}

  // expected-error@+1 {{invalid value '-1'; must be positive}}
  #pragma ii -1
  for (int i=0;i<32;++i) {}

  // expected-error@+1 {{invalid argument of type 'const char [8]'; expected an integer type}}
  #pragma ii "test123"
  for (int i=0;i<32;++i) {}

  #pragma ii 2147483648 // expected-error {{is too large}}
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}foo_max_concurrency
void foo_max_concurrency()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelFPGAMaxConcurrencyAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  #pragma max_concurrency 4
  for (int i=0;i<32;++i) {}

  #pragma max_concurrency // expected-warning {{expected value}}
  for (int i=0;i<32;++i) {}

  #pragma max_concurrency 4 // expected-error {{duplicate Intel FPGA loop attribute}}
  #pragma max_concurrency 8
  for (int i=0;i<32;++i) {}

  #pragma max_concurrency 0
  for (int i=0;i<32;++i) {}

  // expected-warning@+1 {{extra tokens at end of '#pragma max_concurrency' - ignored}}
  #pragma max_concurrency (2) int c[10];

  // expected-warning@+1 {{extra tokens at end of '#pragma max_concurrency' - ignored}}
  #pragma max_concurrency (2) (4)
  for (int i=0;i<32;++i) {}

  // expected-warning@+1 {{extra tokens at end of '#pragma ' - ignored}}
  #pragma max_concurrency (2,4)
  for (int i=0;i<32;++i) {}

  // expected-error@+1 {{invalid value '-1'; must be non-negative}}
  #pragma max_concurrency -1
  for (int i=0;i<32;++i) {}

  // expected-error@+1 {{invalid argument of type 'const char [8]'; expected an integer type}}
  #pragma max_concurrency "test123"
  for (int i=0;i<32;++i) {}

  #pragma max_concurrency 2147483648 // expected-error {{is too large}}
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}foo_max_interleaving
void foo_max_interleaving()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelFPGAMaxInterleavingAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}1
  #pragma max_interleaving 1
  for (int i=0;i<32;++i) {}

  #pragma max_interleaving // expected-warning {{expected value}}
  for (int i=0;i<32;++i) {}

  #pragma max_interleaving 4 // expected-error {{duplicate Intel FPGA loop attribute}}
  #pragma max_interleaving 8
  for (int i=0;i<32;++i) {}

  #pragma max_interleaving 0
  for (int i=0;i<32;++i) {}

  // expected-warning@+1 {{extra tokens at end of '#pragma max_interleaving' - ignored}}
  #pragma max_interleaving (2) int c[10];

  // expected-warning@+1 {{extra tokens at end of '#pragma max_interleaving' - ignored}}
  #pragma max_interleaving (2) (4)
  for (int i=0;i<32;++i) {}

  // expected-warning@+1 {{extra tokens at end of '#pragma ' - ignored}}
  #pragma max_interleaving (2,4)
  for (int i=0;i<32;++i) {}

  // expected-error@+1 {{invalid value '-1'; must be non-negative}}
  #pragma max_interleaving -1
  for (int i=0;i<32;++i) {}

  // expected-error@+1 {{invalid argument of type 'const char [8]'; expected an integer type}}
  #pragma max_interleaving "test123"
  for (int i=0;i<32;++i) {}

  #pragma max_interleaving 2147483648 // expected-error {{is too large}}
  for (int i=0;i<32;++i) {}
}

struct IV_S {
  int arr1[10];
} ivs[20];

//CHECK: FunctionDecl{{.*}}ivdep1
void ivdep1(IV_S* sp)
{
  int i;
  //CHECK: SYCLIntelFPGAIVDepAttr
  //CHECK-NEXT: NULL
  //CHECK-NEXT: MemberExpr{{.*}}arr1
  //CHECK: DeclRefExpr{{.*}}'sp' 'IV_S *'
  #pragma ivdep array(sp->arr1)
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}tivdep 'void (T *)'
//CHECK: SYCLIntelFPGAIVDepAttr
//CHECK-NEXT: NULL
//CHECK-NEXT: MemberExpr{{.*}}arr1
//CHECK: DeclRefExpr{{.*}}'tsp' 'T *'
template <typename T>
void tivdep(T* tsp)
{
  int i;
  #pragma ivdep array(tsp->arr1)
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}t2ivdep 'void (int)'
//CHECK: SYCLIntelFPGAIVDepAttr
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
  //CHECK-NEXT: SYCLIntelFPGAIVDepAttr{{.*}}0
  #pragma ivdep
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelFPGAIVDepAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  #pragma ivdep safelen(4)
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelFPGAIVDepAttr
  //CHECK-NEXT: NULL
  //CHECK-NEXT: DeclRefExpr{{.*}}myArray
  #pragma ivdep array(myArray)
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelFPGAIVDepAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  //CHECK-NEXT: DeclRefExpr{{.*}}myArray
  #pragma ivdep safelen(4) array(myArray)
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelFPGAIVDepAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  //CHECK-NEXT: DeclRefExpr{{.*}}myArray
  //CHECK-NEXT: SYCLIntelFPGAIVDepAttr
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

  #pragma ivdep // expected-note {{previous attribute is here}}
  #pragma ivdep // expected-warning {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen INF}}
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

  #pragma ivdep safelen(4) // expected-warning {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 8 >= safelen 4}}
  #pragma ivdep safelen(8) // expected-note {{previous attribute is here}}
  for (int i=0;i<32;++i) {}

  #pragma ivdep safelen(4) // expected-note {{previous attribute is here}}
  #pragma ivdep safelen(2) // expected-warning {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 4 >= safelen 2}}
  for (int i=0;i<32;++i) {}

  #pragma ivdep safelen(4) // expected-note {{previous attribute is here}}
  #pragma ivdep safelen(4) // expected-warning {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 4 >= safelen 4}}
  for (int i=0;i<32;++i) {}

  #pragma ivdep safelen(8) // expected-warning {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 8}}
  #pragma ivdep            // expected-note {{previous attribute is here}}
  for (int i=0;i<32;++i) {}

  #pragma ivdep // expected-note {{previous attribute is here}}
  #pragma ivdep safelen(8) // expected-warning {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 8}}
  for (int i=0;i<32;++i) {}

  // 2. Partially overlapping pragmas:
  // #pragma ivdep [safelen(n)]
  // #pragma ivdep array(a) [safelen(m)]
  // In this case, the max(n,m) will be used for array a by the optimizer.
  // Thus if m <= n, the "array" pragma is redundant and should trigger a
  // warning.

  //expected-warning@+3 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 4}}
  //expected-note@+1 {{previous attribute is here}}
  #pragma ivdep
  #pragma ivdep array(myArray) safelen(4)
  for (int i=0;i<32;++i) {}

  //expected-warning@+2 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 4}}
  //expected-note@+2 {{previous attribute is here}}
  #pragma ivdep array(myArray) safelen(4)
  #pragma ivdep
  for (int i=0;i<32;++i) {}

  //expected-warning@+3 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 8 >= safelen 4}}
  //expected-note@+1 {{previous attribute is here}}
  #pragma ivdep safelen(8)
  #pragma ivdep array(myArray) safelen(4)
  for (int i=0;i<32;++i) {}

  //expected-warning@+2 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 8 >= safelen 4}}
  //expected-note@+2 {{previous attribute is here}}
  #pragma ivdep array(myArray) safelen(4)
  #pragma ivdep safelen(8)
  for (int i=0;i<32;++i) {}

  //expected-warning@+2 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 8 >= safelen 8}}
  //expected-note@+2 {{previous attribute is here}}
  #pragma ivdep array(myArray) safelen(8)
  #pragma ivdep safelen(8)
  for (int i=0;i<32;++i) {}

  //expected-warning@+3 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 8 >= safelen 8}}
  //expected-note@+1 {{previous attribute is here}}
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
  //expected-warning@+1 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 8 >= safelen 4}}
  #pragma ivdep array(myArray) safelen(4)
  #pragma ivdep array(myArray2) safelen(16)  // this one not redundant
  #pragma ivdep safelen(8)
  //expected-note@-1 {{previous attribute is here}}
  for (int i=0;i<32;++i) {}

  //expected-warning@+3 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen INF}}
  //expected-note@+1 {{previous attribute is here}}
  #pragma ivdep
  #pragma ivdep array(myArray)
  for (int i=0;i<32;++i) {}

  //expected-warning@+2 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen INF}}
  //expected-note@+2 {{previous attribute is here}}
  #pragma ivdep array(myArray)
  #pragma ivdep
  for (int i=0;i<32;++i) {}

  //expected-warning@+6{{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen INF}}
  //expected-note@+6 {{previous attribute is here}}
  //expected-warning@+6 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 8}}
  //expected-note@+4 {{previous attribute is here}}
  //expected-warning@+5 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 16}}
  //expected-note@+2 {{previous attribute is here}}
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
  //CHECK-NEXT: SYCLIntelFPGASpeculatedIterationsAttr
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

  #pragma speculated_iterations 4 // expected-error {{duplicate Intel FPGA loop attribute}}
  #pragma speculated_iterations 8
  for (int i=0;i<32;++i) {}

  // expected-warning@+1 {{extra tokens at end of '#pragma speculated_iterations' - ignored}}
  #pragma speculated_iterations (2) int c[10];

  // expected-warning@+1 {{extra tokens at end of '#pragma speculated_iterations' - ignored}}
  #pragma speculated_iterations (2) (4)
  for (int i=0;i<32;++i) {}

  // expected-warning@+1 {{extra tokens at end of '#pragma ' - ignored}}
  #pragma speculated_iterations (2,4)
  for (int i=0;i<32;++i) {}

  // expected-error@+1 {{invalid argument of type 'const char [8]'; expected an integer type}}
  #pragma speculated_iterations "test123"
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
  for (int i=0;i<32;++i) {}

  #pragma ii 1
  #pragma min_ii_at_target_fmax  // expected-error {{duplicate directives}}
  for (int i=0;i<32;++i) {}

  #pragma ii_at_most  1
  #pragma ii 1  // expected-error {{duplicate directives}}
  for (int i=0;i<32;++i) {}

  #pragma ii_at_least 1
  #pragma min_ii_at_target_fmax  // expected-error {{duplicate directives}}
  for (int i=0;i<32;++i) {}
}

void good();
//CHECK: FunctionDecl{{.*}}foo_disable_loop_pipelining
void foo_disable_loop_pipelining()
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelFPGADisableLoopPipeliningAttr
  #pragma disable_loop_pipelining
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining // expected-error {{duplicate Intel FPGA loop attribute}}
  #pragma disable_loop_pipelining
  for (int i=0;i<32;++i) {}

  // expected-warning@+1 {{extra tokens at end of '#pragma disable_loop_pipelining' - ignored}}
  #pragma disable_loop_pipelining int c[10];

  // expected-warning@+1 {{extra tokens at end of '#pragma disable_loop_pipelining' - ignored}}
  #pragma disable_loop_pipelining 0
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining // expected-error {{not compatible}}
  #pragma max_concurrency 100
  for (int i=0;i<32;++i) {}

  #pragma max_concurrency 100 // expected-error {{not compatible}}
  #pragma disable_loop_pipelining
  for (int i=0;i<32;++i) {}

  //expected-error@+2 {{disable_loop_pipelining and max_concurrency attributes are not compatible}}
  //expected-error@+1 {{disable_loop_pipelining and ii attributes are not compatible}}
  #pragma ii  1
  #pragma max_concurrency 100
  #pragma disable_loop_pipelining
  for (int i=0;i<32;++i) {}

  int i, myArray[10];
  //expected-error@+2 {{disable_loop_pipelining and ivdep attributes are not compatible}}
  //expected-error@+1 {{disable_loop_pipelining and ivdep attributes are not compatible}}
  #pragma disable_loop_pipelining
  #pragma ivdep array(myArray)
  #pragma ivdep safelen(8)
  for (int i=0;i<32;++i) {}

  #pragma ivdep safelen(8) // expected-error {{not compatible}}
  #pragma disable_loop_pipelining
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining // expected-error {{not compatible}}
  #pragma ii 4
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
  #pragma disable_loop_pipelining // expected-error {{incompatible directives}}
  for (int i=0;i<32;++i) {}

  #pragma speculated_iterations 4 // expected-error {{not compatible}}
  #pragma disable_loop_pipelining
  for (int i=0;i<32;++i) {}

  #pragma disable_loop_pipelining // expected-error {{not compatible}}
  #pragma speculated_iterations 4
  for (int i=0;i<32;++i) {}

  //CHECK: good
  good();

  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelFPGADisableLoopPipeliningAttr
  //CHECK: LoopHintAttr{{.*}}UnrollCount
  //CHECK-NEXT: IntegerLiteral{{.*}}2
  //CHECK: ForStmt
  #pragma disable_loop_pipelining
  #pragma unroll(2)
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK: LoopHintAttr{{.*}}UnrollCount
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  //CHECK: SYCLIntelFPGADisableLoopPipeliningAttr
  //CHECK: ForStmt
  #pragma unroll(4)
  #pragma disable_loop_pipelining
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
  //CHECK: SubstNonTypeTemplateParmExpr{{.*}} 'int'
  //CHECK-NEXT: NonTypeTemplateParmDecl{{.*}} referenced 'int' depth 0 index 0 size
  //CHECK-NEXT: IntegerLiteral{{.*}}5
  #pragma ii_at_least  size
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}nontypeargument1
template <int size>
void nontypeargument1()
{
  // CHECK: AttributedStmt
  // CHECK-NEXT: SYCLIntelFPGASpeculatedIterationsAttr
  // CHECK: SubstNonTypeTemplateParmExpr{{.*}} 'int'
  // CHECK-NEXT: NonTypeTemplateParmDecl{{.*}} referenced 'int' depth 0 index 0 size
  // CHECK-NEXT: IntegerLiteral{{.*}}2
  #pragma speculated_iterations size
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}nontypeargument2
template <int size>
void nontypeargument2()
{
   // CHECK: AttributedStmt
   // CHECK-NEXT: SYCLIntelFPGAMaxConcurrencyAttr
   // CHECK: SubstNonTypeTemplateParmExpr{{.*}} 'int'
   // CHECK-NEXT: NonTypeTemplateParmDecl{{.*}} referenced 'int' depth 0 index 0 size
   // CHECK-NEXT: IntegerLiteral{{.*}}4
  #pragma max_concurrency size
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}nontypeargument3
template <int size>
void nontypeargument3()
{
  // CHECK: AttributedStmt
  // CHECK-NEXT: SYCLIntelFPGAMaxInterleavingAttr
  // CHECK: SubstNonTypeTemplateParmExpr{{.*}} 'int'
  // CHECK-NEXT: NonTypeTemplateParmDecl{{.*}} referenced 'int' depth 0 index 0 size
  // CHECK-NEXT: IntegerLiteral{{.*}} 6
  #pragma max_interleaving size
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}nontypeargument4
template <int size>
void nontypeargument4()
{
 // CHECK: AttributedStmt
 // CHECK-NEXT: SYCLIntelFPGALoopCoalesceAttr
 // CHECK: SubstNonTypeTemplateParmExpr{{.*}} 'int'
 // CHECK-NEXT: NonTypeTemplateParmDecl{{.*}} referenced 'int' depth 0 index 0 size
 // CHECK-NEXT: IntegerLiteral{{.*}}8
  #pragma loop_coalesce size
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}nontypeargument5
template <int size>
void nontypeargument5()
{
  // CHECK: AttributedStmt
  // CHECK-NEXT: SYCLIntelFPGAIIAttr{{.*}}
  // CHECK: SubstNonTypeTemplateParmExpr{{.*}} 'int'
  // CHECK-NEXT: NonTypeTemplateParmDecl{{.*}} referenced 'int' depth 0 index 0 size
  // CHECK-NEXT: IntegerLiteral{{.*}}10
  #pragma ii size
  for (int i=0;i<32;++i) {}
}

int main()
{
  nontypeargument<5>();
  nontypeargument1<2>();
  nontypeargument2<4>();
  nontypeargument3<6>();
  nontypeargument4<8>();
  nontypeargument5<10>();
}

//CHECK: FunctionDecl{{.*}}do_stuff
template<int LEN>
int do_stuff(int N) {
  int temp = 0;
  // CHECK: AttributedStmt
  // CHECK-NEXT: SYCLIntelFPGAIVDepAttr
  // CHECK-NEXT: NULL
  // CHECK-NEXT: DeclRefExpr{{.*}}NonTypeTemplateParm{{.*}} 'LEN' 'int'
  // CHECK: FunctionDecl{{.*}} do_stuff
  // CHECK-NEXT: TemplateArgument integral 5
  // CHECK: AttributedStmt
  // CHECK-NEXT: SYCLIntelFPGAIVDepAttr{{.*}} 5
  // CHECK-NEXT: SubstNonTypeTemplateParmExpr{{.*}} 'int'
  // CHECK-NEXT: NonTypeTemplateParmDecl{{.*}} referenced 'int' depth 0 index 0 LEN
  // CHECK-NEXT: IntegerLiteral{{.*}}5
#pragma ivdep safelen(LEN)
  for (int i = 0; i < N; ++i) {
    temp += i;
  }
  return temp;
}

int dut() {
  return do_stuff<5>(10);
}

//CHECK: FunctionDecl{{.*}}test 'void (long *)'
void test(long* buffer1)
{
  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelFPGAIVDepAttr
  //CHECK-NEXT: NULL
  //CHECK-NEXT: DeclRefExpr{{.*}}'buffer1' 'long *'
  //CHECK-NEXT: WhileStmt
  //CHECK-NEXT: ImplicitCastExpr{{.*}}'bool' <IntegralToBoolean>
  //CHECK_NEXT: IntegerLiteral{{.*}} 1
  #pragma ivdep array(buffer1)
  while (1) { }
}

#ifdef TRIGGER_ERROR
template <int A, int B, int C>
void max_concurrency_dependent() {
  int a[10];
  // expected-error@+1 {{'max_concurrency' attribute requires a non-negative integral compile time constant expression}}
  #pragma max_concurrency C
  for (int i = 0; i != 10; ++i)
    a[i] = 0;

  // expected-error@+1 {{duplicate Intel FPGA loop attribute 'max_concurrency'}}
  #pragma max_concurrency A
  #pragma max_concurrency B
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
}

template <int A, int B, int C>
void max_interleaving_dependent() {
  int a[10];
  // expected-error@+1 {{'max_interleaving' attribute requires a non-negative integral compile time constant expression}}
  #pragma max_interleaving C
  for (int i = 0; i != 10; ++i)
    a[i] = 0;

  // expected-error@+1 {{duplicate Intel FPGA loop attribute 'max_interleaving'}}
  #pragma max_interleaving A
  #pragma max_interleaving B
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
}

template <int A, int B, int C>
void ii_dependent() {
  int a[10];
  // expected-error@+1 {{'ii' attribute requires a positive integral compile time constant expression}}
  #pragma ii C
  for (int i = 0; i != 10; ++i)
    a[i] = 0;

  // expected-error@+1 {{duplicate Intel FPGA loop attribute 'ii'}}
  #pragma ii A
  #pragma ii B
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
}

template <int A, int B, int C>
void speculated_iterations_dependent() {
  int a[10];
  // expected-error@+1 {{'speculated_iterations' attribute requires a non-negative integral compile time constant expression}}
  #pragma speculated_iterations C
  for (int i = 0; i != 10; ++i)
    a[i] = 0;

  // expected-error@+1 {{duplicate Intel FPGA loop attribute 'speculated_iterations'}}
  #pragma speculated_iterations A
  #pragma speculated_iterations B
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
}

template <int A, int B, int C>
void loop_coalesce_dependent() {
  int a[10];
  // expected-error@+1 {{'loop_coalesce' attribute requires a positive integral compile time constant expression}}
  #pragma loop_coalesce C
  for (int i = 0; i != 10; ++i)
    a[i] = 0;

  // expected-error@+1 {{duplicate Intel FPGA loop attribute 'loop_coalesce'}}
  #pragma loop_coalesce A
  #pragma loop_coalesce B
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
}

int test() {
  max_concurrency_dependent<1, 4, -2>();
  //expected-note@-1 +{{in instantiation of function template specialization}}
  max_interleaving_dependent<1, 6, -4>();
  //expected-note@-1 +{{in instantiation of function template specialization}}
  ii_dependent<2, 4, -1>();
  //expected-note@-1 +{{in instantiation of function template specialization}}
  speculated_iterations_dependent<1, 4, -3>();
  //expected-note@-1 +{{in instantiation of function template specialization}}
  loop_coalesce_dependent<3, 6, -4>();
  //expected-note@-1 +{{in instantiation of function template specialization}}
  return 0;
}
#endif
