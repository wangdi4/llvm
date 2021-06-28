// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -ferror-limit 100 %s -Wuninitialized

void bar();

void foo()
{
  int v;
  int x = 0, expr = 0;
  int val = 1;
  bool B = true;
  int d = 2, e = 3;

  // expected-error@+1 {{directive '#pragma omp atomic' cannot contain more than one 'capture' clause}}
  #pragma omp atomic compare capture capture
  { v=x; if(x < expr) { x = expr; } }

  // expected-error@+1 {{directive '#pragma omp atomic' cannot contain more than one 'compare' clause}}
  #pragma omp atomic compare capture compare
  { v=x; if(x < expr) {x = expr;} }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}' or '{cond-update-stmt v = x;}' where x is an lvalue expression with scalar type}}
  // expected-note@+2 {{expected compound statement}}
  #pragma omp atomic compare capture
  v = x;

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}' or '{cond-update-stmt v = x;}' where x is an lvalue expression with scalar type}}
  // expected-note@+2 {{expected exactly two expression statements}}
  #pragma omp atomic compare capture
  {v = x;}

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}' or '{cond-update-stmt v = x;}' where x is an lvalue expression with scalar type}}
  // expected-note@+2 {{expected exactly two expression statements}}
  #pragma omp atomic compare capture
  {v=x; if(x < expr) {x = expr;} v=x;}

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}' or '{cond-update-stmt v = x;}' where x is an lvalue expression with scalar type}}
  // expected-note@+2 {{expected if statement}}
  #pragma omp atomic compare capture
  {v=x; v=x;}

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}' or '{cond-update-stmt v = x;}' where x is an lvalue expression with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  {if(x < expr) { x = expr; } if(x < expr) { x = expr;}}

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}' or '{cond-update-stmt v = x;}' where x is an lvalue expression with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  {bar(); if(x < expr) { x = expr; } }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}' or '{cond-update-stmt v = x;}' where x is an lvalue expression with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  {if(x < expr) { x = expr; } bar();}

  int x1 = 0;
  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}' or '{cond-update-stmt v = x;}' where x is an lvalue expression with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare capture
  {v=x1; if(x < expr) { x = expr;} }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}' or '{cond-update-stmt v = x;}' where x is an lvalue expression with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare capture
  {if(x < expr) { x = expr; } v=x1;}
}
// end INTEL_COLLAB
