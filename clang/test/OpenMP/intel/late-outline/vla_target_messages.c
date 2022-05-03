// INTEL_COLLAB
//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc \
//RUN:  -disable-llvm-passes -fopenmp -fopenmp-targets=spir64    \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility -Werror      \
//RUN:  -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir64 -emit-llvm -disable-llvm-passes   \
//RUN:  -fopenmp -fopenmp-targets=spir64 -fopenmp-late-outline -fopenmp-typed-clauses    \
//RUN:  -fintel-compatibility -fopenmp-is-device -verify -o - %s   \
//RUN:  -fopenmp-host-ir-file-path %t_host.bc -Wsource-uses-openmp

void foo(int nx, int ny) {
  float x[nx][ny];

  // expected-error@+2 {{variable length arrays are not supported for the current target}}
  #pragma omp target
  #pragma omp teams firstprivate (x) // not ok, because we cannot firstprivatize
                                     // a VLA in teams
  {
    x[0][0] = 1.0f;
  }

  // expected-error@+2 {{variable length arrays are not supported for the current target}}
  #pragma omp target
  #pragma omp teams distribute firstprivate (x)
  for (int i=0; i<nx; ++i)
  {
    x[i][0] = 1.0f;
  }

  // expected-error@+2 {{variable length arrays are not supported for the current target}}
  #pragma omp target
  #pragma omp teams distribute simd firstprivate (x)
  for (int i=0; i<nx; ++i)
  {
    x[i][0] = 1.0f;
  }

  // expected-error@+2 {{variable length arrays are not supported for the current target}}
  #pragma omp target
  #pragma omp teams distribute parallel for simd firstprivate (x)
  for (int i=0; i<nx; ++i)
  {
    x[i][0] = 1.0f;
  }

  // expected-error@+2 {{variable length arrays are not supported for the current target}}
  #pragma omp target
  #pragma omp teams distribute parallel for firstprivate (x)
  for (int i=0; i<nx; ++i)
  {
    x[i][0] = 1.0f;
  }

  // expected-error@+2 {{variable length arrays are not supported for the current target}}
  #pragma omp target
  #pragma omp teams loop firstprivate (x)
  for (int i=0; i<nx; ++i)
  {
    x[i][0] = 1.0f;
  }

  // expected-error@+2 {{variable length arrays are not supported for the current target}}
  #pragma omp target
  #pragma omp teams private(x) // not ok, VLA in private clause in teams
  {
    x[0][0] = 1.0f;
  }
  // expected-error@+3 {{cannot generate code for reduction on variable length array}}
  // expected-note@+2 {{variable length arrays are not supported for the current target}}
  #pragma omp target
  #pragma omp teams reduction(+:x) // not ok, VLA in reduction clause
  {
    x[0][0] = 1.0f;
  }
}
// end INTEL_COLLAB
