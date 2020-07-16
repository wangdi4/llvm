// RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

void __attribute((max_concurrency(4))) foo1()
{
  int A;
}
// CHECK: FunctionDecl{{.*}}foo1
// CHECK: MaxConcurrencyAttr
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: value: Int 4
// CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}

void __attribute((max_concurrency(0))) foo2()
{
  int A;
}
// CHECK: FunctionDecl{{.*}}foo2
// CHECK: MaxConcurrencyAttr
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: value: Int 0
// CHECK-NEXT: IntegerLiteral{{.*}}0{{$}}

//expected-error@+1{{requires integer constant between 0 and 1048576}}
void __attribute((max_concurrency(-1))) bar1()
{
}

//expected-error@+1{{requires integer constant between 0 and 1048576}}
void __attribute((max_concurrency(2000000))) bar2()
{
}

//expected-error@+1{{'max_concurrency' attribute takes one argument}}
void __attribute((max_concurrency(32,64))) bar3()
{
}

template <int tvalue>
void __attribute((max_concurrency(tvalue))) tfoo2()
{
}

void call()
{
  // CHECK: FunctionDecl{{.*}}tfoo2
  // CHECK: MaxConcurrencyAttr
  // CHECK-NEXT: DeclRefExpr{{.*}}NonTypeTemplateParm{{.*}}tvalue
  // CHECK: FunctionDecl{{.*}}tfoo2
  // CHECK-NEXT: TemplateArgument integral 8
  // CHECK: MaxConcurrencyAttr
  // CHECK-NEXT: ConstantExpr
  // CHECK-NEXT: value: Int 8
  // CHECK-NEXT: SubstNonTypeTemplateParmExpr
  // CHECK-NEXT: NonTypeTemplateParmDecl
  // CHECK-NEXT: IntegerLiteral{{.*}}8{{$}}
  tfoo2<8>();
}
