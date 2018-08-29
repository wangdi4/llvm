// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only %s -verify -ast-dump -pedantic | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility-enable=PragmaBlockLoop -fsyntax-only %s -verify -ast-dump -pedantic | FileCheck %s

void foo(int i, int *x, int *y) {
  // CHECK: AttributedStmt
  // CHECK-NEXT: IntelBlockLoopAttr{{.*}}
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma block_loop
  for (i = 0; i < 10; ++i) {  // this is OK
    x[i] = y[i];
  }
  // expected-error@+2 {{expected a for, while, or do-while loop to follow '#pragma block_loop'}}
  #pragma block_loop
  i = 7;
  for (i = 0; i < 10; ++i) {  // this is OK
    x[i] = y[i];
  }
}

void bar(int i, int *x, int *y) {

  // CHECK: AttributedStmt
  // CHECK-NEXT: IntelBlockLoopAttr{{.*}}
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma block_loop
  for (i = 0; i < 10; ++i) {  // this is OK
    x[i] = y[i];
  }
}

void zoo(int i, int *x, int *y) {

  // CHECK: AttributedStmt
  // CHECK-NEXT: IntelBlockLoopAttr{{.*}} Implicit 2 3
  // CHECK-NEXT: IntegerLiteral{{.*}} 16
  // CHECK-NEXT: IntegerLiteral{{.*}} 16
  // CHECK-NEXT: ForStmt
  #pragma block_loop level(2:3) factor(16)
  for (i = 0; i < 10; ++i) {  // this is OK
    x[i] = y[i];
  }
}

void error_check(int i, int *x, int *y) {
  // expected-error@+1 {{overlapping blockloop levels between level(2:3) and level(1:3)}}
  #pragma block_loop level(2:3) factor(16)
  // expected-note@+1 {{second block_loop level is specified here}}
  #pragma block_loop level(1:3) factor(16)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  // expected-error@+2 {{overlapping blockloop levels between level(-1:-1) and level(2:3)}}
  // expected-note@+2 {{second block_loop level is specified here}}
  #pragma block_loop factor(16)
  #pragma block_loop level(2:3) factor(16)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  // expected-error@+1 {{invalid block level: value must be less than or equal to 8}}
  #pragma block_loop level(1:9) factor(16)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  // expected-error@+1 {{invalid block level: value must be greater than zero}}
  #pragma block_loop level(0:5) factor(16)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  // expected-error@+1 {{invalid block level: the first parameter must be less than the second}}
  #pragma block_loop level(5:3) factor(16)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  int array[10];
  // expected-error@+1 {{invalid argument of type 'int [10]'; expected a scalar type}}
  #pragma block_loop level(3) private(array)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  // expected-error@+1 {{invalid argument of type 'int [10]'; expected an integer type}}
  #pragma block_loop level(3) factor(array)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  double m;
  // expected-error@+1 {{invalid argument of type 'double'; expected an integer type}}
  #pragma block_loop level(3) factor(m)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  int a;
  // expected-error@+1 {{only variables can be arguments to '#pragma block_loop private}}
  #pragma block_loop level(3) private(a*3)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  // expected-error@+3 {{duplicate use of variable name "'a'"}}
  // expected-note@+1 {{previously referenced here}}
  #pragma block_loop level(3) private(a)
  #pragma block_loop level(2) private(a)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  // expected-note@+1 {{declared here}}
  int var;
  // expected-error@+2 {{expression is not an integral constant expression}}
  // expected-note@+1 {{read of non-const variable 'var' is not allowed in a constant expression}}
  #pragma block_loop level(3) private(var) factor(var)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
}

template <typename T>
void zoo(T var) // expected-note {{declared here}}
{
  int var1, i;
  // expected-error@+2 {{expression is not an integral constant expression}}
  // expected-note@+1 {{read of non-const variable 'var' is not allowed in a constant expression}}
  #pragma block_loop private(var, var) factor(var)
  for (i = 0; i < 10; ++i)
    var1 = var1 + i;

  // expected-error@+2 {{duplicate use of variable name "'var'"}}
  // expected-note@+1 {{previously referenced here}}
  #pragma block_loop private(var, var)
  for (i = 0; i < 10; ++i)
    var1 = var1 + i;
}

template <typename T>
void bar1(T var)
{
  int i;
  // expected-error@+1 {{invalid argument of type 'double'; expected an integer type}}
  #pragma block_loop private(var) factor(var)
  for (i = 0; i < 10; ++i)
    var = var + i;
}

template <typename T>
void bar2(T var)
{
  int i;
  // expected-error@+1 {{only variables can be arguments to '#pragma block_loop private'}}
  #pragma block_loop private(var * 3)
  for (i = 0; i < 10; ++i)
    var = var + i;
}

template <int size>
void nontypeargument(int var)
{
  int i;
  // expected-error@+1 {{only variables can be arguments to '#pragma block_loop private'}}
  #pragma block_loop private(var * 3)
  for (i = 0; i < 10; ++i)
    var = var + i;
}

//expected-note@+1 {{declared here}}
void bar3(int var)
{
  int i;
  // expected-error@+2 {{expression is not an integral constant expression}}
  // expected-note@+1 {{read of non-const variable 'var' is not allowed in a constant expression}}
  #pragma block_loop level(var:var)
  for (i = 0; i < 10; ++i)
    var = var + i;
}

int main()
{
// expected-note@+1 {{in instantiation of function template specialization 'zoo<int>' requested here}}
zoo<int>(10);
// expected-note@+1 {{in instantiation of function template specialization 'bar1<double>' requested here}}
bar1<double>(10.0);
// expected-note@+1 {{in instantiation of function template specialization 'bar2<int>' requested here}}
bar2<int>(10);
// expected-note@+1 {{in instantiation of function template specialization 'nontypeargument<100>' requested here}}
nontypeargument<100>(10);
}
