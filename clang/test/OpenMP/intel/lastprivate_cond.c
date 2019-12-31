// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-version=50 -fintel-compatibility -fopenmp-late-outline -triple x86_64-unknown-linux-gnu %s | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -o - -DERRS -fopenmp -fopenmp-version=50 -fintel-compatibility -fopenmp-late-outline -triple x86_64-unknown-linux-gnu -verify %s

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
  Foo agg; //expected-note {{'agg' defined here}}
  unsigned int var = 0;
  int avar;

  #pragma omp parallel for lastprivate(foo:avar) //expected-error {{expected 'conditional' in OpenMP clause 'lastprivate'}}
  for (var = 0; var < 6; var++) {}

  #pragma omp parallel for lastprivate(conditional:agg) //expected-error {{expected 'scalar' or 'vector' in OpenMP clause 'lastprivate' with conditional modifier}}
  for (var = 0; var < 6; var++) {}
}
#endif
