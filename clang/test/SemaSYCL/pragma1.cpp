//RUN: %clang_cc1 -triple spir64-unknown-unknown-sycldevice -disable-llvm-passes -fsycl-is-device -ast-dump -verify -pedantic %s | FileCheck %s

struct IV_S {
  int arr1[10];
} ivs[20];

//CHECK: FunctionDecl{{.*}}ivdep1
void ivdep1(IV_S* sp)
{
  int i;
  //CHECK: SYCLIntelIVDepAttr{{.*}}
  //CHECK-NEXT: NULL
  //CHECK-NEXT: MemberExpr{{.*}}arr1
  //CHECK: DeclRefExpr{{.*}}'sp' 'IV_S *'
  [[intel::ivdep(sp->arr1)]]
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}tivdep 'void (T *)'
//CHECK: SYCLIntelIVDepAttr{{.*}}
//CHECK-NEXT: MemberExpr{{.*}}arr1
//CHECK: DeclRefExpr{{.*}}'tsp' 'T *'
//CHECK-NEXT: NULL
template <typename T>
void tivdep(T* tsp)
{
  int i;
  [[intel::ivdep(tsp->arr1)]]
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}t2ivdep 'void (int)'
//CHECK: SYCLIntelIVDepAttr{{.*}}
//CHECK-NEXT: NULL
//CHECK-NEXT: MemberExpr{{.*}}arr1
//CHECK: DeclRefExpr{{.*}}'lsp' 'IV_S *'
template <int item>
void t2ivdep(int i) {
  IV_S *lsp = &ivs[item];
  [[intel::ivdep(lsp->arr1)]]
  for (int i=0;i<32;++i) {}
}

//CHECK: FunctionDecl{{.*}}foo_ivdep
void foo_ivdep()
{
  int myArray[40];
  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelIVDepAttr{{.*}}0
  [[intel::ivdep()]]
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelIVDepAttr{{.*}}
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  [[intel::ivdep(4)]]
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelIVDepAttr{{.*}}
  //CHECK-NEXT: NULL
  //CHECK-NEXT: DeclRefExpr{{.*}}myArray
  [[intel::ivdep(myArray)]]
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelIVDepAttr{{.*}}
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  //CHECK-NEXT: DeclRefExpr{{.*}}myArray
  [[intel::ivdep(myArray,4)]]
  for (int i=0;i<32;++i) {}

  //CHECK: AttributedStmt
  //CHECK-NEXT: SYCLIntelIVDepAttr{{.*}}
  //CHECK-NEXT: IntegerLiteral{{.*}}4
  //CHECK-NEXT: DeclRefExpr{{.*}}myArray
  //CHECK-NEXT: SYCLIntelIVDepAttr{{.*}}
  //CHECK-NEXT: NULL
  //CHECK-NEXT: DeclRefExpr{{.*}}dArray
  double dArray[42];
  [[intel::ivdep(myArray,4)]]
  [[intel::ivdep(dArray)]]
  for (int i=0;i<32;++i) {}

  IV_S* p;
  tivdep(p);
  t2ivdep<4>(0);

  //okay now
  [[intel::ivdep(4)]]
  [[intel::ivdep(myArray,8)]]
  for (int i=0;i<32;++i) {}

  [[intel::ivdep()]] // expected-note {{previous attribute is here}}
  [[intel::ivdep()]] // expected-warning {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen INF}}
  for (int i=0;i<32;++i) {}

  //expected-error@+1{{duplicate argument to 'ivdep'; attribute requires one or both of a safelen and array}}
  [[intel::ivdep(4,8)]]
  for (int i=0;i<32;++i) {}

  //expected-error@+1{{duplicate argument to 'ivdep'; attribute requires one or both of a safelen and array}}
  [[intel::ivdep(myArray, myArray)]]
  for (int i=0;i<32;++i) {}

  //expected-error@+1{{use of undeclared identifier 'typo_array'}}
  [[intel::ivdep(typo_array)]]
  for (int i=0;i<32;++i) {}

  IV_S *mysp = &ivs[5];
  //expected-error@+1{{no member named 'lala'}}
  [[intel::ivdep(mysp->lala)]]
  for (int i=0;i<32;++i) {}

  [[intel::ivdep(4)]] // expected-warning {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 8 >= safelen 4}}
  [[intel::ivdep(8)]] // expected-note {{previous attribute is here}}
  for (int i=0;i<32;++i) {}

  [[intel::ivdep(4)]] // expected-note {{previous attribute is here}}
  [[intel::ivdep(2)]] // expected-warning {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 4 >= safelen 2}}
  for (int i=0;i<32;++i) {}

  [[intel::ivdep(4)]] // expected-note {{previous attribute is here}}
  [[intel::ivdep(4)]] // expected-warning {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 4 >= safelen 4}}
  for (int i=0;i<32;++i) {}

  [[intel::ivdep(8)]] // expected-warning {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 8}}
  [[intel::ivdep()]]  // expected-note {{previous attribute is here}}
  for (int i=0;i<32;++i) {}

  [[intel::ivdep()]] // expected-note {{previous attribute is here}}
  [[intel::ivdep(8)]] // expected-warning {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 8}}
  for (int i=0;i<32;++i) {}

  //expected-warning@+3 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 4}}
  //expected-note@+1 {{previous attribute is here}}
  [[intel::ivdep()]]
  [[intel::ivdep(myArray, 4)]]
  for (int i=0;i<32;++i) {}

  //expected-warning@+2 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 4}}
  //expected-note@+2 {{previous attribute is here}}
  [[intel::ivdep(myArray, 4)]]
  [[intel::ivdep()]]
  for (int i=0;i<32;++i) {}

  //expected-warning@+3 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 8 >= safelen 4}}
  //expected-note@+1 {{previous attribute is here}}
  [[intel::ivdep(8)]]
  [[intel::ivdep(myArray, 4)]]
  for (int i=0;i<32;++i) {}

  //expected-warning@+2 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 8 >= safelen 4}}
  //expected-note@+2 {{previous attribute is here}}
  [[intel::ivdep(myArray, 4)]]
  [[intel::ivdep(8)]]
  for (int i=0;i<32;++i) {}

  //expected-warning@+2 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 8 >= safelen 8}}
  //expected-note@+2 {{previous attribute is here}}
  [[intel::ivdep(myArray, 8)]]
  [[intel::ivdep(8)]]
  for (int i=0;i<32;++i) {}

  //expected-warning@+3 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 8 >= safelen 4}}
  //expected-note@+1 {{previous attribute is here}}
  [[intel::ivdep(8)]]
  [[intel::ivdep(myArray, 4)]]
  for (int i=0;i<32;++i) {}

  [[intel::ivdep(myArray, 6)]]
  [[intel::ivdep(4)]] // not redundant directive since  m > n
  for (int i=0;i<32;++i) {}

  [[intel::ivdep(4)]] // not redundant directive since  m > n
  [[intel::ivdep(myArray, 6)]]
  for (int i=0;i<32;++i) {}

  int myArray2[24];
  //expected-warning@+1 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 8 >= safelen 4}}
  [[intel::ivdep(myArray, 4)]]
  [[intel::ivdep(myArray2, 16)]]  // this one not redundant
  [[intel::ivdep(8)]]
  //expected-note@-1 {{previous attribute is here}}
  for (int i=0;i<32;++i) {}

  //expected-warning@+3 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen INF}}
  //expected-note@+1 {{previous attribute is here}}
  [[intel::ivdep()]]
  [[intel::ivdep(myArray)]]
  for (int i=0;i<32;++i) {}

  //expected-warning@+2 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen INF}}
  //expected-note@+2 {{previous attribute is here}}
  [[intel::ivdep(myArray)]]
  [[intel::ivdep()]]
  for (int i=0;i<32;++i) {}

  //expected-warning@+6{{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen INF}}
  //expected-note@+6 {{previous attribute is here}}
  //expected-warning@+6 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 8}}
  //expected-note@+4 {{previous attribute is here}}
  //expected-warning@+5 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 16}}
  //expected-note@+2 {{previous attribute is here}}
  [[intel::ivdep(myArray)]]
  [[intel::ivdep()]]
  [[intel::ivdep(8)]]
  [[intel::ivdep(myArray, 16)]]
  for (int i=0;i<32;++i) {}

}

//CHECK: FunctionDecl{{.*}}do_stuff
template<int LEN>
int do_stuff(int N) {
  int temp = 0;
  // CHECK: AttributedStmt
  // CHECK-NEXT: SYCLIntelIVDepAttr {{.*}}
  // CHECK-NEXT: DeclRefExpr{{.*}}NonTypeTemplateParm{{.*}} 'LEN' 'int'
  // CHECK-NEXT: NULL
  // CHECK: FunctionDecl{{.*}} do_stuff
  // CHECK-NEXT: TemplateArgument integral 5
  // CHECK: AttributedStmt
  // CHECK-NEXT: SYCLIntelIVDepAttr{{.*}} 5
  // CHECK-NEXT: SubstNonTypeTemplateParmExpr{{.*}} 'int'
  // CHECK-NEXT: NonTypeTemplateParmDecl{{.*}} referenced 'int' depth 0 index 0 LEN
  // CHECK-NEXT: IntegerLiteral{{.*}}5
  [[intel::ivdep(LEN)]]
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
  //CHECK-NEXT: SYCLIntelIVDepAttr{{.*}}
  //CHECK-NEXT: NULL
  //CHECK-NEXT: DeclRefExpr{{.*}}'buffer1' 'long *'
  //CHECK-NEXT: WhileStmt
  //CHECK-NEXT: ImplicitCastExpr{{.*}}'bool' <IntegralToBoolean>
  //CHECK-NEXT: IntegerLiteral{{.*}} 1
  [[intel::ivdep(buffer1)]]
  while (1) { }
}
