// RUN: %clang_cc1 -fopenmp -fopenmp-late-outline %s -Wuninitialized -verify

int arr[100];
void bar();

void foo()
{
  int i;
  int z;

  // A list item may not appear in a lastprivate clause unless it is the loop
  // iteration variable of a loop that is associated with the construct

  // expected-error@+1 {{only loop iteration variables are allowed in 'lastprivate' clause in 'omp loop' directives}}
  #pragma omp target parallel loop bind(thread) lastprivate(z)
  for (i=0; i<1000; ++i) {
    z = i+11;
  }

  // The only constructs that may be nested inside a loop region are the loop
  // construct, the parallel construct, the simd construct, and combined
  // constructs for which the first construct is a parallel construct.

  #pragma omp target parallel loop bind(teams)
  for (i=0; i<10; ++i) {

    // expected-error@+1 {{region cannot be nested inside 'target parallel loop' region}}
    #pragma omp task
    {
      arr[i] = i+11;
    }
  }
}
