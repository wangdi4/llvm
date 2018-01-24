// RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

void __attribute((max_concurrency(4))) foo1()
{
  int A;
}
// CHECK: FunctionDecl{{.*}}foo1
// CHECK: MaxConcurrencyAttr{{.*}}4

void __attribute((max_concurrency(0))) foo2()
{
  int A;
}
// CHECK: FunctionDecl{{.*}}foo2
// CHECK: MaxConcurrencyAttr{{.*}}0

void __attribute((max_concurrency(-1))) bar1() // expected-error {{'max_concurrency' attribute requires integer constant between 0 and 1048576}}
{
}

void __attribute((max_concurrency(2000000))) bar2() // expected-error {{'max_concurrency' attribute requires integer constant between 0 and 1048576}}
{
}

void __attribute((max_concurrency(32,64))) bar3() // expected-error {{'max_concurrency' attribute takes one argument}}
{
}
