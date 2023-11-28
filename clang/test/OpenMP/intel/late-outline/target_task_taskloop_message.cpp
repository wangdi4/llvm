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

#pragma omp target parallel
// expected-error@+1 {{region 'task' is not yet supported inside 'target parallel' region}}
#pragma omp task
  {
    k++;
  }

#pragma omp target parallel
// expected-error@+1 {{region 'taskloop' is not yet supported inside 'target parallel' region}}
#pragma omp taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target parallel
// expected-error@+1 {{region 'taskloop simd' is not yet supported inside 'target parallel' region}}
#pragma omp taskloop simd
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target parallel
// expected-error@+1 {{region 'master taskloop' is not yet supported inside 'target parallel' region}}
#pragma omp master taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target parallel
// expected-error@+1 {{region 'master taskloop simd' is not yet supported inside 'target parallel' region}}
#pragma omp master taskloop simd
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target parallel
#pragma omp master
// expected-error@+1 {{region 'taskloop' is not yet supported inside 'target parallel' region}}
#pragma omp taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target parallel
#pragma omp single
// expected-error@+1 {{region 'task' is not yet supported inside 'target parallel' region}}
#pragma omp task
  {
    for (int l = 0; l < 16; l++) {
      k++;
    }
  }

#pragma omp target teams
// expected-error@+1 {{region cannot be closely nested inside 'target teams' region; perhaps you forget to enclose 'omp task' directive into a parallel region?}}
#pragma omp task
  {
    k++;
  }

#pragma omp target teams
// expected-error@+1 {{region cannot be closely nested inside 'target teams' region; perhaps you forget to enclose 'omp taskloop' directive into a parallel region?}}
#pragma omp taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target teams
// expected-error@+1 {{region cannot be closely nested inside 'target teams' region; perhaps you forget to enclose 'omp taskloop simd' directive into a parallel region?}}
#pragma omp taskloop simd
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target teams
// expected-error@+1 {{region cannot be closely nested inside 'target teams' region; perhaps you forget to enclose 'omp master taskloop' directive into a parallel region?}}
#pragma omp master taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target teams
// expected-error@+1 {{region cannot be closely nested inside 'target teams' region; perhaps you forget to enclose 'omp master taskloop simd' directive into a parallel region?}}
#pragma omp master taskloop simd
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target teams
// expected-error@+1 {{region cannot be closely nested inside 'target teams' region; perhaps you forget to enclose 'omp master' directive into a parallel region?}}
#pragma omp master
// expected-error@+1 {{region 'taskloop' is not yet supported inside 'target teams' region}}
#pragma omp taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target teams
// expected-error@+1 {{region cannot be closely nested inside 'target teams' region; perhaps you forget to enclose 'omp single' directive into a parallel region?}}
#pragma omp single
// expected-error@+1 {{region 'task' is not yet supported inside 'target teams' region}}
#pragma omp task
  {
    for (int l = 0; l < 16; l++) {
      k++;
    }
  }

#pragma omp target teams distribute
// expected-error@+1 {{region 'task' is not yet supported inside 'target teams distribute' region}}
#pragma omp task
  {
    k++;
  }

#pragma omp target teams distribute
// expected-error@+1 {{region 'taskloop' is not yet supported inside 'target teams distribute' region}}
#pragma omp taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target teams distribute
// expected-error@+1 {{region 'taskloop simd' is not yet supported inside 'target teams distribute' region}}
#pragma omp taskloop simd
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target teams distribute
// expected-error@+1 {{region 'master taskloop' is not yet supported inside 'target teams distribute' region}}
#pragma omp master taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target teams distribute
// expected-error@+1 {{region 'master taskloop simd' is not yet supported inside 'target teams distribute' region}}
#pragma omp master taskloop simd
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target teams distribute
#pragma omp master
// expected-error@+1 {{region 'taskloop' is not yet supported inside 'target teams distribute' region}}
#pragma omp taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target teams distribute
#pragma omp single
// expected-error@+1 {{region 'task' is not yet supported inside 'target teams distribute' region}}
#pragma omp task
  {
    for (int l = 0; l < 16; l++) {
      k++;
    }
  }

#pragma omp teams
// expected-error@+1 {{region cannot be closely nested inside 'teams' region; perhaps you forget to enclose 'omp task' directive into a parallel region?}}
#pragma omp task
  {
    k++;
  }

#pragma omp teams 
// expected-error@+1 {{region cannot be closely nested inside 'teams' region; perhaps you forget to enclose 'omp taskloop' directive into a parallel region?}}
#pragma omp taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp teams
// expected-error@+1 {{region cannot be closely nested inside 'teams' region; perhaps you forget to enclose 'omp taskloop simd' directive into a parallel region?}}
#pragma omp taskloop simd
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp teams
// expected-error@+1 {{region cannot be closely nested inside 'teams' region; perhaps you forget to enclose 'omp master taskloop' directive into a parallel region?}}
#pragma omp master taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp teams
// expected-error@+1 {{region cannot be closely nested inside 'teams' region; perhaps you forget to enclose 'omp master taskloop simd' directive into a parallel region?}}
#pragma omp master taskloop simd
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp teams
// expected-error@+1 {{region cannot be closely nested inside 'teams' region; perhaps you forget to enclose 'omp master' directive into a parallel region?}}
#pragma omp master
// expected-error@+1 {{region 'taskloop' is not yet supported inside 'teams' region}}
#pragma omp taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp teams
// expected-error@+1 {{region cannot be closely nested inside 'teams' region; perhaps you forget to enclose 'omp single' directive into a parallel region?}}
#pragma omp single
// expected-error@+1 {{region 'task' is not yet supported inside 'teams' region}}
#pragma omp task
  {
    for (int l = 0; l < 16; l++) {
      k++;
    }
  }

#pragma omp teams distribute
// expected-error@+1 {{region 'task' is not yet supported inside 'teams distribute' region}}
#pragma omp task
  {
    k++;
  }

#pragma omp teams distribute
// expected-error@+1 {{region 'taskloop' is not yet supported inside 'teams distribute' region}}
#pragma omp taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp teams distribute
// expected-error@+1 {{region 'taskloop simd' is not yet supported inside 'teams distribute' region}}
#pragma omp taskloop simd
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp teams distribute
// expected-error@+1 {{region 'master taskloop' is not yet supported inside 'teams distribute' region}}
#pragma omp master taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp teams distribute
// expected-error@+1 {{region 'master taskloop simd' is not yet supported inside 'teams distribute' region}}
#pragma omp master taskloop simd
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp teams distribute
#pragma omp master
// expected-error@+1 {{region 'taskloop' is not yet supported inside 'teams distribute' region}}
#pragma omp taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp teams distribute
#pragma omp single
// expected-error@+1 {{region 'task' is not yet supported inside 'teams distribute' region}}
#pragma omp task
  {
    for (int l = 0; l < 16; l++) {
      k++;
    }
  }

#pragma omp target teams distribute simd
// expected-error@+1 {{OpenMP constructs may not be nested inside a simd region except for ordered simd, simd, scan, or atomic directive}}
#pragma omp task
  {
    k++;
  }

#pragma omp target teams distribute simd
// expected-error@+1 {{OpenMP constructs may not be nested inside a simd region except for ordered simd, simd, scan, or atomic directive}}
#pragma omp taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp teams distribute simd
// expected-error@+1 {{OpenMP constructs may not be nested inside a simd region except for ordered simd, simd, scan, or atomic directive}}
#pragma omp task
  {
    k++;
  }

#pragma omp teams distribute simd
// expected-error@+1 {{OpenMP constructs may not be nested inside a simd region except for ordered simd, simd, scan, or atomic directive}}
#pragma omp taskloop
  for (int l = 0; l < 16; l++) {
    k++;
  }

#pragma omp target parallel for
  for (int l = 0; l < 16; l++) {
// expected-error@+1 {{region 'task' is not yet supported inside 'target parallel for' region}}
#pragma omp task
    {
      k++;
    }
  }

#pragma omp target parallel for
  for (int l = 0; l < 16; l++) {
// expected-error@+1 {{region 'taskloop' is not yet supported inside 'target parallel for' region}}
#pragma omp taskloop
    for (int m = 0; m < 16; m++) {
      k++;
    }
  }

#pragma omp target teams distribute parallel for
  for (int l = 0; l < 16; l++) {
// expected-error@+1 {{region 'task' is not yet supported inside 'target teams distribute parallel for' region}}
#pragma omp task
    {
      k++;
    }
  }

#pragma omp target teams distribute parallel for
  for (int l = 0; l < 16; l++) {
// expected-error@+1 {{region 'taskloop' is not yet supported inside 'target teams distribute parallel for' region}}
#pragma omp taskloop
    for (int m = 0; m < 16; m++) {
      k++;
    }
  }

#pragma omp teams distribute  parallel for
  for (int l = 0; l < 16; l++) {
// expected-error@+1 {{region 'task' is not yet supported inside 'teams distribute parallel for' region}}
#pragma omp task
    {
      k++;
    }
  }

#pragma omp teams distribute parallel for
  for (int l = 0; l < 16; l++) {
// expected-error@+1 {{region 'taskloop' is not yet supported inside 'teams distribute parallel for' region}}
#pragma omp taskloop
    for (int m = 0; m < 16; m++) {
      k++;
    }
  }

#pragma omp target teams distribute parallel for simd
  for (int l = 0; l < 16; l++) {
// expected-error@+1 {{OpenMP constructs may not be nested inside a simd region except for ordered simd, simd, scan, or atomic directive}}
#pragma omp task
    {
      k++;
    }
  }

#pragma omp target teams distribute parallel for simd
  for (int l = 0; l < 16; l++) {
// expected-error@+1 {{OpenMP constructs may not be nested inside a simd region except for ordered simd, simd, scan, or atomic directive}}
#pragma omp taskloop
    for (int m = 0; m < 16; m++) {
      k++;
    }
  }

#pragma omp teams distribute  parallel for simd
  for (int l = 0; l < 16; l++) {
// expected-error@+1 {{OpenMP constructs may not be nested inside a simd region except for ordered simd, simd, scan, or atomic directive}}
#pragma omp task
    {
      k++;
    }
  }

#pragma omp teams distribute parallel for simd
  for (int l = 0; l < 16; l++) {
// expected-error@+1 {{OpenMP constructs may not be nested inside a simd region except for ordered simd, simd, scan, or atomic directive}}
#pragma omp taskloop
    for (int m = 0; m < 16; m++) {
      k++;
    }
  }
}
