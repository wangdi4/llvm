// RUN: %clang_cc1 -fintel-compatibility -verify %s

double foo1(double i, double j, double k) {
  return __generic(i, j, k, 4, 5, 6, 7, 8, 9);
}

double foo2(double i, double j, double k) {
  return __generic(i, j, k, 4, 5, 6, 7, 8); // expected-error{{expected ','}}
}

double foo3(double i, double j, double k) {
  return __generic(i, j, k, , 5, 6, 7, 8, 9); // expected-error{{expected expression}}
    // expected-warning@-1 {{non-void function 'foo3' should return a value}} 
}

double foo4(double *i) {
  return __generic(i, , , 4, 5, 6, 7, 8, 9); // expected-error{{expression must have an arithmetic type}}
}
