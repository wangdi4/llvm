// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s
// RUN: %clang_cc1 -fintel-compatibility-enable=PragmaNoVector -fintel-compatibility-enable=PragmaVector -fsyntax-only -verify %s

void foo(int lb, int ub, int *a, int *b) {
#pragma novector (enable) // expected-warning {{extra tokens at end of '#pragma novector' - ignored}}
  for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
}

void foo1(int lb, int ub, int *a, int *b) {
#pragma vector (enable) // expected-warning {{extra tokens at end of '#pragma vector' - ignored}}
  for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
}
void foo2(int lb, int ub, int *a, int *b) { // expected-note {{declared here}}
#pragma vector enable // expected-warning {{invalid option 'enable'; expected always}}
  for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }

#pragma vector dynamic_align (x) // expected-warning {{extra tokens at end of '#pragma vector' - ignored}}
  for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }

#pragma vector always assert (x) // expected-warning {{extra tokens at end of '#pragma vector' - ignored}}
  for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
#pragma vector assert dynamic_align  // expected-warning {{missing option; always must precede assert qualifier - ignored}}
  for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
#pragma vector vector always assert // expected-warning {{invalid option 'vector'; expected always - ignored}}
  for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
#pragma vector always vector assert // expected-warning {{invalid option 'vector'; expected always - ignored}}
  for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
#pragma vector temporal() // expected-warning {{extra tokens at end of '#pragma vector' - ignored}}
  for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
// expected-error@+2 {{expression is not an integral constant expression}}
// expected-note@+1 {{function parameter 'lb' with unknown value cannot be used in a constant expression}}
#pragma vector vectorlength(lb)
  for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
#pragma vector vectorlength(2:3) // expected-warning {{extra tokens at end of '#pragma vector' - ignored}}
  for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
#pragma vector vectorlength // expected-warning {{missing '(' after '#pragma vector vectorlength' - ignoring}}
  for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
}
