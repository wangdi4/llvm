// RUN: %clang_cc1 -std=c++11 -verify -fopenmp-late-outline \
// RUN:  -fopenmp -ferror-limit 200 %s

void foo()
{
  int k = 0;
#pragma omp target
// expected-error@+1 {{region 'task' is not yet supported inside 'target' region}}
#pragma omp task
  {
    k++;
  }

#pragma omp target
// expected-error@+1 {{region 'taskloop' is not yet supported inside 'target' region}}
#pragma omp taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target
// expected-error@+1 {{region 'taskloop simd' is not yet supported inside 'target' region}}
#pragma omp taskloop simd
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target
// expected-error@+1 {{region 'master taskloop' is not yet supported inside 'target' region}}
#pragma omp master taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target
// expected-error@+1 {{region 'master taskloop simd' is not yet supported inside 'target' region}}
#pragma omp master taskloop simd
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target
#pragma omp master
// expected-error@+1 {{region 'taskloop' is not yet supported inside 'target' region}}
#pragma omp taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target
#pragma omp single
// expected-error@+1 {{region 'task' is not yet supported inside 'target' region}}
#pragma omp task
  {
    for (int l = 0; l < 16; l++) {
      k++;
    }
  }
}
