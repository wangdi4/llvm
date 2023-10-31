// RUN: %libomptarget-compile-run-and-check-generic
// RUN: %libomptarget-compileopt-run-and-check-generic

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  long unsigned **DP = 0;
  int N = 128;
  int Threads = 128;
  int Teams = 440;

#pragma omp target map(from : DP)
  DP = (long unsigned **)malloc(sizeof(long unsigned *) * Threads * Teams);

#if INTEL_CUSTOMIZATION
#pragma omp target teams distribute parallel for num_teams(Teams)              \
    thread_limit(Threads) is_device_ptr(DP)
#else // INTEL_CUSTOMIZATION
#pragma omp target teams distribute parallel for num_teams(Teams)              \
    thread_limit(Threads)
#endif // INTEL_CUSTOMIZATION
  for (int i = 0; i < Threads * Teams; ++i)
    DP[i] = (long unsigned *)malloc(sizeof(long unsigned) * N);

#if INTEL_CUSTOMIZATION
#pragma omp target teams distribute parallel for num_teams(Teams)              \
    thread_limit(Threads) is_device_ptr(DP)
#else // INTEL_CUSTOMIZATION
#pragma omp target teams distribute parallel for num_teams(Teams)              \
    thread_limit(Threads)
#endif // INTEL_CUSTOMIZATION
  for (int i = 0; i < Threads * Teams; ++i) {
    for (int j = 0; j < N; ++j) {
      DP[i][j] = i + j;
    }
  }

  long unsigned s = 0;
#if INTEL_CUSTOMIZATION
#pragma omp target teams distribute parallel for num_teams(Teams)              \
    thread_limit(Threads) reduction(+ : s) is_device_ptr(DP)
#else // INTEL_CUSTOMIZATION
#pragma omp target teams distribute parallel for num_teams(Teams)              \
    thread_limit(Threads) reduction(+ : s)
#endif // INTEL_CUSTOMIZATION
  for (int i = 0; i < Threads * Teams; ++i) {
    for (int j = 0; j < N; ++j) {
      s += DP[i][j];
    }
  }

  // CHECK: Sum: 203458478080
  printf("Sum: %li\n", s);
  return 0;
}
