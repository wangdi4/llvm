// CQ#366312
// RUN: %clang_cc1 -fintel-compatibility %s -verify %s

typedef struct foo {
  short field1;
  long field2;
  char *field3;
} foo_t;

void check() {

  short *p1;
  int *p2;
  foo_t *p3;
  char *p4;
  double *p5;
  int i;
  __int64 i64;

  // OK as sizeof(char) <  sizeof(short *)
  (char)p1 *= i; // expected-warning{{assignment to lvalue cast that does not lengthen the size of an object is an Intel Extension}}
  // OK as sizeof(double *) == sizeof(short *)
  (double *)p1 += (int)p2; // expected-warning{{assignment to lvalue cast that does not lengthen the size of an object is an Intel Extension}}
  // NOT OK as sizeof(long long) > sizeof(short)
  (long long)*p1 %= i; // expected-error{{assignment to cast is illegal, lvalue casts are not supported}}
  // NOT OK as sizeof(__int64) > sizeof(char)
  (__int64)*p4 = i64; // expected-error{{assignment to cast is illegal, lvalue casts are not supported}}
  // OK as sizeof(short *) == sizeof(short *)
  short tmp = *((short *)p3)++; // expected-warning{{assignment to lvalue cast that does not lengthen the size of an object is an Intel Extension}}
  // OK as sizeof(unsigned int) <= sizeof(__int64)
  (unsigned int)i64 = 5; // expected-warning{{assignment to lvalue cast that does not lengthen the size of an object is an Intel Extension}}
  // OK as sizeof(int *) == sizeof(short *)
  ((int *)p3)++; // expected-warning{{assignment to lvalue cast that does not lengthen the size of an object is an Intel Extension}}
  // OK as sizeof(int) <= sizeof(double)
  (int)*p5++ = 5; // expected-warning{{assignment to lvalue cast that does not lengthen the size of an object is an Intel Extension}}
  // OK as sizeof((char *) == sizeof(int *)
  *((char *)p2)++ = 5; // expected-warning{{assignment to lvalue cast that does not lengthen the size of an object is an Intel Extension}}

}
