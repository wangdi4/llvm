// RUN: %clang_cc1  -fopenmp  -x c++ -fopenmp-late-outline \
// RUN:  -triple x86_64-pc-linux-gnu  -emit-llvm %s -o - \
// RUN:  -fopenmp-targets=spir64 \
// RUN:  | FileCheck %s --check-prefix=CHECK

// expected-no-diagnostics
#ifndef HEADER
#define HEADER

template<typename tx>
tx ftemplate(int n) {
  tx a = 0;

  #pragma omp target teams ompx_register_alloc_mode(small)
  {
  }

  short b = 1;
  #pragma omp target teams num_teams(b) ompx_register_alloc_mode(default)
  {
    a += b;
  }

  return a;
}

static
int fstatic(int n) {

  #pragma omp target teams distribute parallel for simd num_teams(n) ompx_register_alloc_mode(auto)
  for (int i = 0; i < n ; ++i) {
  }

  #pragma omp target teams ompx_register_alloc_mode(large) nowait
  {
  }

  return n+1;
}

struct S1 {
  double a;

  int r1(int n){
    int b = 1;

    #pragma omp target teams ompx_register_alloc_mode(auto)
    {
      this->a = (double)b + 1.5;
    }

    #pragma omp target ompx_register_alloc_mode(default)
    {
      this->a = 2.5;
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

#endif

// CHECK-LABEL: define {{.*}}@_ZN2S12r1Ei
// CHECK-NEXT:  entry:
// CHECK:        "DIR.OMP.TARGET"
// CHECK-SAME:   "QUAL.OMP.REGISTER.ALLOC.MODE"(i32 1)
// CHECK:        "DIR.OMP.END.TARGET"
// CHECK:        "DIR.OMP.TARGET"
// CHECK-SAME:   "QUAL.OMP.REGISTER.ALLOC.MODE"(i32 0)
// CHECK:        "DIR.OMP.END.TARGET"

// CHECK-LABEL: define {{.*}}@_ZL7fstatici
// CHECK-NEXT:  entry:
// CHECK:        "DIR.OMP.TARGET"
// CHECK-SAME:   "QUAL.OMP.REGISTER.ALLOC.MODE"(i32 1)
// CHECK:        "DIR.OMP.END.TARGET"
// CHECK:        "DIR.OMP.TARGET"
// CHECK-SAME:   "QUAL.OMP.REGISTER.ALLOC.MODE"(i32 3)
// CHECK:        "DIR.OMP.END.TARGET"

// CHECK-LABEL: define {{.*}}@_Z9ftemplateIiET_i
// CHECK-NEXT:  entry:
// CHECK:        "DIR.OMP.TARGET"
// CHECK-SAME:   "QUAL.OMP.REGISTER.ALLOC.MODE"(i32 2)
// CHECK:        "DIR.OMP.END.TARGET"
