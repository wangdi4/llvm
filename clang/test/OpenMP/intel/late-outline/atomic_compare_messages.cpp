// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -fopenmp-late-outline \
// RUN:  -ferror-limit 100 %s -Wuninitialized

void bar();

void foo()
{
  int x = 0, expr = 0;
  int val = 1;
  bool B = true;
  int d = 2, e = 3;

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{expected an if statement or assignment expression}}
  #pragma omp atomic compare
  bar();

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{expected a conditional operator}}
  #pragma omp atomic compare
  x = expr;

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{expected a '<', '>', or '==' expression}}
  #pragma omp atomic compare
  x = B ? x : expr;

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{expected a '<', '>', or '==' expression}}
  #pragma omp atomic compare
  x = x <= expr ? x : expr;

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare
  x = expr < val ? x : expr;

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare
  x = x == e ? d : val;

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare
  x = e == x ? d : x;

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{expected a '<', '>', or '==' expression}}
  #pragma omp atomic compare
  if (expr) { x = expr; }

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{expected compound statement containing one assignment}}
  #pragma omp atomic compare
  if (x < expr) bar();

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{expected compound statement containing one assignment}}
  #pragma omp atomic compare
  if (x < expr) { x = expr; val++; }

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{unexpected else in if statement}}
  #pragma omp atomic compare
  if (x < expr) { x = expr; } else { val++; }

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{expected an assignment expression}}
  #pragma omp atomic compare
  if (x < expr) { bar(); }

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{expected an assignment expression}}
  #pragma omp atomic compare
  if (x < expr) { x += expr; }

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{expected a '<', '>', or '==' expression}}
  #pragma omp atomic compare
  if (x <= expr) { x = expr; }

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare
  if (expr == x) { x = expr; }

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare
  if (val < expr) { x = expr; }

}
// end INTEL_COLLAB
