// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -fopenmp-late-outline \
// RUN:  -ferror-limit 100 %s -Wuninitialized

void bar();

struct NotScalar { };

void foo()
{
  struct NotScalar A, B;
  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{expected expression of scalar type}}
  #pragma omp atomic compare
  A = B;

}

struct St {
  int x;
  int y;
  int expr;
};
int arr[2] = { 1,2 };

void test() {

  struct St s = {1,2,3};

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare
  s.x = s.expr < s.x ? s.expr : s.y;

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare
  s.x = s.expr < s.y ? s.expr : s.x;

  // expected-error@+3 {{the statement for 'atomic compare' must be 'x = expr ordop x ? expr : x;', 'x = x ordop expr ? expr : x;', 'x = x == e ? d : x;', 'if(expr ordop x) {x=expr;}', 'if(x ordop expr) {x=expr;}', or 'if(x==e) {x=d;}' where x is an lvalue with scalar type}}
  // expected-note@+2 {{cannot match 'x' component in expression}}
  #pragma omp atomic compare
  if (s.expr < arr[0]) { arr[1] = s.expr; }
}

 register int rix __asm__("esp");

 void test_reg()
 {
   int expr;
   // expected-error@+2 {{global register variable not supported in 'atomic compare'}}
   #pragma omp atomic compare
   rix = expr < rix ? expr : rix;
 }
// end INTEL_COLLAB
