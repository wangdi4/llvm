// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -o - %s -DERRS -fopenmp -fintel-compatibility -fintel-openmp -triple x86_64-unknown-linux-gnu -verify

int main()
{
    int glob = 0;
    unsigned int var = 0;
    { var++;
      #pragma omp parallel
      {
//CHECK: QUAL.OMP.LASTPRIVATE:CONDITIONAL
        #pragma omp for lastprivate(conditional:glob) schedule(static,1)
        for (var = 0; var < 6; var++) {
          if (var%3==1) {
            glob = 2*var;
          }
        }
      }
    }
    if (glob == 8) {
      return 0;
    } else {
      return !0;
    }
}

#ifdef ERRS
typedef struct Foo { int i; double d; } Foo;
void bar()
{
  Foo agg;
  unsigned int var = 0;
  int avar;

  #pragma omp parallel for lastprivate(foo:avar) //expected-error {{incorrect lastprivate modifier, expected 'conditional'}}
  for (var = 0; var < 6; var++) {}

  #pragma omp parallel for lastprivate(conditional:agg) //expected-error {{expected 'scalar' in OpenMP clause 'lastprivate' with conditional modifier}}
  for (var = 0; var < 6; var++) {}
}
#endif
