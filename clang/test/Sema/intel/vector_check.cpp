// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only %s -verify -pedantic
// RUN: %clang_cc1 -fintel-compatibility-enable=PragmaNoVector -fintel-compatibility-enable=PragmaVector -fsyntax-only %s -verify -pedantic
// expected error

void foo()
{
  int a[10];
// expected-error@+2 {{duplicate directives '#pragma novector' and '#pragma vector'}}
  #pragma novector
  #pragma vector
  for (int i = 1; i <10; i++) a[i]=1;
}
void bar()
{
  int a[10];
// expected-error@+2 {{duplicate directives '#pragma vector' and '#pragma vector'}}
  #pragma vector
  #pragma vector
  for (int i = 1; i <10; i++) a[i]=1;
}
void zoo()
{
  int a[10];
// expected-error@+1 {{duplicate directives '#pragma vector' and '#pragma vector'}}
  #pragma vector always always
  for (int i = 1; i <10; i++) a[i]=1;
}
