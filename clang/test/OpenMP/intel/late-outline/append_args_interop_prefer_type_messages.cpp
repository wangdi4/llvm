// INTEL_COLLAB
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux -fopenmp -fopenmp-version=51 -std=c++20 -o - %s

typedef void *omp_interop_t;

int Other;

class A {
public:
  void memberfoo_v1(float *A, float *B, int *I, omp_interop_t IOp);

  //expected-error@+2 {{prefer_list item must be a string literal or constant integral expression}}
  #pragma omp declare variant(memberfoo_v1) match(construct={dispatch}) \
    append_args(interop(target, prefer_type(1.0, 'a')))
  void memberbar1(float *A, float *B, int *I) { return; }

  //expected-error@+4 {{expected interop type: 'target' and/or 'targetsync'}}
  //expected-error@+3 {{expected ')'}}
  //expected-note@+2 {{to match this '('}}
  #pragma omp declare variant(memberfoo_v1) match(construct={dispatch}) \
    append_args(interop(target, prefer_type("opencl","sycl"), prefer_type(1,2)))
  void memberbar2(float *A, float *B, int *I) { return; }

  //expected-error@+2 {{expected interop type: 'target' and/or 'targetsync'}}
  #pragma omp declare variant(memberfoo_v1) match(construct={dispatch}) \
    append_args(interop(prefer_type("opencl")))
  void memberbar3(float *A, float *B, int *I) { return; }
};

void foo();

template <void (*d)()>
class X {
public:
  void tfoo_v1(float *A, float *B, int *I, omp_interop_t IOp);

  //expected-error@+2 {{prefer_list item must be a string literal or constant integral expression}}
  #pragma omp declare variant(tfoo_v1) match(construct={dispatch}) \
    append_args(interop(target, prefer_type(1,d)))
  void tbar(float *A, float *B, int *I) { return; }
};

int main()
{
  A a;
  float f1 = 0.0;
  int I = 0;
  //expected-note@+1 {{in instantiation of template class 'X<&foo>' requested here}}
  X<foo> x;
  x.tbar(&f1, &f1, &I);
  return 0;
}

// end INTEL_COLLAB
