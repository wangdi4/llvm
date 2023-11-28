// RUN: %clang_cc1 -verify -fopenmp -ferror-limit 100 %s -Wuninitialized

template<typename tx>
tx ftemplate(int n) {
  tx a = 0;

  // expected-error@+1  {{expected '(' after 'ompx_register_alloc_mode'}}
  #pragma omp target teams ompx_register_alloc_mode
  {
  }

  short b = 1;
  // expected-error@+1  {{expected 'default', 'auto', 'small' or 'large' in OpenMP clause 'ompx_register_alloc_mode'}}
  #pragma omp target teams num_teams(b) ompx_register_alloc_mode(xxx)
  {
    a += b;
  }

  return a;
}

static
int fstatic(int n) {

  // expected-error@+3  {{expected ')'}}
  // expected-note@+2 {{to match this '('}}
  // expected-error@+1  {{expected 'default', 'auto', 'small' or 'large' in OpenMP clause 'ompx_register_alloc_mode'}}
  #pragma omp target teams distribute parallel for simd num_teams(n) ompx_register_alloc_mode(
  for (int i = 0; i < n ; ++i) {
  }
  // expected-error@+3 {{expected ')'}}
  // expected-note@+2 {{to match this '('}}
  // expected-error@+1 {{expected 'default', 'auto', 'small' or 'large' in OpenMP clause 'ompx_register_alloc_mode'}}
  #pragma omp target teams ompx_register_alloc_mode(,) nowait
  {
  }
  // expected-error@+1 {{directive '#pragma omp target teams' cannot contain more than one 'ompx_register_alloc_mode' clause}}
  #pragma omp target teams ompx_register_alloc_mode(auto) ompx_register_alloc_mode(default) nowait
  {
  }
  return n+1;
}

struct S1 {
  double a;

  int r1(int n){
    int b = 1;

  // expected-error@+1  {{expected 'default', 'auto', 'small' or 'large' in OpenMP clause 'ompx_register_alloc_mode'}}
    #pragma omp target teams ompx_register_alloc_mode(b)
    {
      this->a = (double)b + 1.5;
    }

    return (int)a;
  }
};

int bar(int n){
  int a = 0;

  S1 S;
  a += S.r1(n);

  a += fstatic(n);

  a += ftemplate<int>(n);

  return a;
}
