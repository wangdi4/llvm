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
// expected-error@+1 {{duplicate directives '#pragma vector always' and '#pragma vector always'}}
  #pragma vector always always
  for (int i = 1; i <10; i++) a[i]=1;

// expected-error@+1 {{duplicate directives '#pragma vector aligned' and '#pragma vector aligned'}}
  #pragma vector aligned aligned
  for(int j=1; j<10; j++) { a[j]=1; }

// expected-error@+1 {{duplicate directives '#pragma vector dynamic_align' and '#pragma vector dynamic_align'}}
#pragma vector dynamic_align dynamic_align
  for(int j=1; j<10; j++) { a[j]=1; }

#pragma vector dynamic_align
#pragma vector dynamic_align // expected-error {{duplicate directives '#pragma vector dynamic_align' and '#pragma vector dynamic_align'}}
  for(int j=1; j<10; j++) { a[j]=1; }

#pragma vector dynamic_align
#pragma vector nodynamic_align // expected-error {{duplicate directives '#pragma vector dynamic_align' and '#pragma vector nodynamic_align'}}
  for(int j=1; j<10; j++) { a[j]=1; }

#pragma vector vecremainder
#pragma vector vecremainder // expected-error {{duplicate directives '#pragma vector vecremainder' and '#pragma vector vecremainder'}}
  for(int j=1; j<10; j++) { a[j]=1; }

#pragma vector vecremainder
#pragma vector novecremainder // expected-error {{duplicate directives '#pragma vector vecremainder' and '#pragma vector novecremainder'}}
  for(int j=1; j<10; j++) { a[j]=1; }
#pragma vector temporal nontemporal // expected-error {{duplicate directives '#pragma vector temporal' and '#pragma vector nontemporal'}}
  for(int j=1; j<10; j++) { a[j]=1; }
#pragma vector vectorlength(-2,9) // expected-error {{invalid value '-2'; must be positive}}
  for(int j=1; j<10; j++) { a[j]=1; }
}
