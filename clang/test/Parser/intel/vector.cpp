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
void foo2(int lb, int ub, int *a, int *b) {
#pragma vector enable // expected-warning {{invalid option 'enable'; expected always}}
  for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }

#pragma vector dynamic_align (x) // expected-warning {{extra tokens at end of '#pragma vector' - ignored}}
  for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }

}
