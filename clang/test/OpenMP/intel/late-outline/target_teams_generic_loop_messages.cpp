// RUN: %clang_cc1 -fopenmp -fopenmp-version=50 -fopenmp-late-outline %s -Wuninitialized -verify

int arr[100];
void bar();

void foo()
{
  int i;
  int z;

  // Test message specific to 'teams loop' only.

  // A list item may not appear in a lastprivate clause unless it is the loop
  // iteration variable of a loop that is associated with the construct

  // expected-error@+1 {{only loop iteration variables are allowed in 'lastprivate' clause in 'omp loop' directives}}
  #pragma omp target teams loop bind(thread) lastprivate(z)
  for (i=0; i<1000; ++i) {
    z = i+11;
  }
}
