// RUN: %clang_cc1 -fopenmp %s -Wuninitialized -verify

int arr[100];
void bar();

void foo()
{
  int i;
  int z;

  // A list item may not appear in a lastprivate clause unless it is the loop
  // iteration variable of a loop that is associated with the construct

  // expected-error@+1 {{only loop iteration variables are allowed in 'lastprivate' clause in loop directives}}
  #pragma omp loop bind(thread) lastprivate(z)
  for (i=0; i<1000; ++i) {
    z = i+11;
  }

  // If a loop construct is not nested inside another OpenMP construct and it
  // appears in a procedure, the bind clause must be present.

  // expected-error@+1 {{'loop' directive without 'bind' clause must be nested in another construct}}
  #pragma omp loop
  for (i=0; i<10; ++i) {
    arr[i] = i+11;
  }

  // If the bind clause is present and binding is teams, the loop region
  // corresponding to the loop construct must be strictly nested inside a
  // teams region.

  #pragma omp target
  {
    // expected-error@+1 {{region cannot be closely nested inside 'target' region; perhaps you forget to enclose 'omp loop' directive into a teams region?}}
    #pragma omp loop bind(teams)
    for (i=0; i<10; ++i) {
      arr[i] = i+11;
    }
  }

  // The only constructs that may be nested inside a loop region are the loop
  // construct, the parallel construct, the simd construct, and combined
  // constructs for which the first construct is a parallel construct.

  #pragma omp loop bind(teams)
  for (i=0; i<10; ++i) {

    // expected-error@+1 {{region cannot be nested inside 'loop' region}}
    #pragma omp task
    {
      arr[i] = i+11;
    }
  }

  #pragma omp parallel
  {
    //  expected-error@+1 {{expected 'teams', 'parallel' or 'thread' in OpenMP clause 'bind'}}
    #pragma omp loop bind(tea)
    for (i=0; i<10; ++i) {
      arr[i] = i+11;
    }

    //  expected-error@+1 {{expected '(' after 'bind'}}
    #pragma omp loop bind
    for (i=0; i<10; ++i) {
      arr[i] = i+11;
    }
  }
}
