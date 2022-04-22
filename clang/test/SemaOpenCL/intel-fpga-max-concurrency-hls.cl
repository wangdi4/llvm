// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s
// RUN: %clang_cc1 -x cl -triple x86_64-unknown-unknown-intelfpga -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

void __attribute((max_concurrency(4))) foo1(void)
{
  int A;
}
// CHECK: FunctionDecl{{.*}}foo1
// CHECK: MaxConcurrencyAttr
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: value: Int 4
// CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}

void __attribute((max_concurrency(0))) foo2(void)
{
  int A;
}
// CHECK: FunctionDecl{{.*}}foo2
// CHECK: MaxConcurrencyAttr
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: value: Int 0
// CHECK-NEXT: IntegerLiteral{{.*}}0{{$}}

//expected-error@+1{{requires integer constant between 0 and 1048576}}
void __attribute((max_concurrency(-1))) bar1(void)
{
}

//expected-error@+1{{requires integer constant between 0 and 1048576}}
void __attribute((max_concurrency(2000000))) bar2(void)
{
}

//expected-error@+1{{'max_concurrency' attribute takes one argument}}
void __attribute((max_concurrency(32,64))) bar3(void)
{
}
