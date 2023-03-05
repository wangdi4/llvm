// RUN: %libomptarget-compile-run-and-check-generic

#include <omp.h>
#include <stdio.h>

int main() {
  const int N = 64;

#if INTEL_CUSTOMIZATION
  int *hst_ptr = omp_alloc(N * sizeof(int), omp_target_host_mem_alloc);
#else // INTEL_CUSTOMIZATION
  int *hst_ptr = omp_alloc(N * sizeof(int), llvm_omp_target_host_mem_alloc);
#endif // INTEL_CUSTOMIZATION

  for (int i = 0; i < N; ++i)
    hst_ptr[i] = 2;

#pragma omp target teams distribute parallel for map(tofrom : hst_ptr[0 : N])
  for (int i = 0; i < N; ++i)
    hst_ptr[i] -= 1;

  int sum = 0;
  for (int i = 0; i < N; ++i)
    sum += hst_ptr[i];

<<<<<<< HEAD
#if INTEL_CUSTOMIZATION
  omp_free(hst_ptr, omp_target_shared_mem_alloc);
#else // INTEL_CUSTOMIZATION
  omp_free(hst_ptr, llvm_omp_target_shared_mem_alloc);
#endif // INTEL_CUSTOMIZATION
=======
  omp_free(hst_ptr, llvm_omp_target_host_mem_alloc);
>>>>>>> 5d560b6966b722e45085f010bb98d2b081c461c7
  // CHECK: PASS
  if (sum == N)
    printf("PASS\n");
}
