// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -fopenmp-late-outline \
// RUN:  -ferror-limit 100 %s -Wuninitialized

void bar();

void foo()
{
  int v;
  int x = 0, expr = 0;
  int val = 1;
  bool B = true;
  int d = 2, e = 3;
  int r;

  // expected-error@+1 {{directive '#pragma omp atomic' cannot contain more than one 'capture' clause}}
  #pragma omp atomic compare capture capture
  { v=x; if(x < expr) { x = expr; } }

  // expected-error@+1 {{directive '#pragma omp atomic' cannot contain more than one 'compare' clause}}
  #pragma omp atomic compare capture compare
  { v=x; if(x < expr) {x = expr;} }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected compound or if statement}}
  #pragma omp atomic compare capture
  v = x;

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected exactly two expression statements}}
  #pragma omp atomic compare capture
  {v = x;}

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected exactly two expression statements}}
  #pragma omp atomic compare capture
  {v=x; if(x < expr) {x = expr;} v=x;}

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected if statement}}
  #pragma omp atomic compare capture
  {v=x; v=x;}

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  {if(x < expr) { x = expr; } if(x < expr) { x = expr;}}

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  {bar(); if(x < expr) { x = expr; } }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  {if(x < expr) { x = expr; } bar();}

  int x1 = 0;
  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare capture
  {v=x1; if(x < expr) { x = expr;} }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare capture
  {if(x < expr) { x = expr; } v=x1;}

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected compound statement}}
  #pragma omp atomic compare capture
  if (x==e) x = d; else v = x;

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected else in if statement}}
  #pragma omp atomic compare capture
  if (x==e) { x = d; }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  if (x==e) { x = d; x = d; } else { v = x; }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  if (x==e) { } else { v = x; }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  if (x==e) { x = d; } else { v = x; v = x; }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  if (x==e) { x = d; } else { }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  if (x==e) { x++; } else { v = x; }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  if (x==e) { x+=d; } else { v = x; }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected '==' operator}}
  #pragma omp atomic compare capture
  if (!x) { x = d; } else { v = x; }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected '==' operator}}
  #pragma omp atomic compare capture
  if (x<e) { x = d; } else { v = x; }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected an lvalue expression}}
  #pragma omp atomic compare capture
  if (1==e) { x = d; } else { v = x; }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare capture
  if (e==x) { x = d; } else { v = x; }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  if (x==e) { x = d; } else { v++; }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  if (x==e) { x = d; } else { v+=x; }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare capture
  if (x==e) { x = d; } else { v = d; }

  // Correct forms:
  // { r = x == e; if (r) { x = d; } }
  // { r = x == e; if (r) { x = d; } else { v = x; } }

  float BadR;
  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{result must be of integral type}}
  #pragma omp atomic compare capture
  { BadR = x == e; if (BadR) { x = d; } }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected '==' operator}}
  #pragma omp atomic compare capture
  { r = 1; if (r) { x = d; } }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected '==' operator}}
  #pragma omp atomic compare capture
  { r = x < e; if (r) { x = d; } }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected an lvalue expression}}
  #pragma omp atomic compare capture
  { r = 1 == e; if (r) { x = d; } }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{cannot match 'r' component in expression}}
  #pragma omp atomic compare capture
  { r = x == e; if (x) { x = d; } }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{cannot match 'r' component in expression}}
  #pragma omp atomic compare capture
  { r = x == e; if (1) { x = d; } }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected compound statement}}
  #pragma omp atomic compare capture
  { r = x == e; if (r) x = d; }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  { r = x == e; if (r) { bar(); } }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  { r = x == e; if (r) { } }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  { r = x == e; if (r) { x = d; x = d; } }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare capture
  { r = x == e; if (r) { v = d; } }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  { r = x == e; if (r) { x = d; } else { } }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{expected assignment expression}}
  #pragma omp atomic compare capture
  { r = x == e; if (r) { x = d; } else { v = x; v = x; } }

  // expected-error@+3 {{the statement for 'atomic compare capture' must be '{v=x; cond-update-stmt}', '{cond-update-stmt v = x;}', 'if(x==e) {x=d;} else {v=x;}', '{r=x==e; if(r) {x=d;}}' or '{r=x==e; if(r) {x=d;} else {v=x;}}' where x, r, and v are lvalue expressions with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare capture
  { r = x == e; if (r) { x = d; } else { v = 1; } }

}
// end INTEL_COLLAB
