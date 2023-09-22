// RUN: %libomptarget-compile-generic && \
// RUN: %libomptarget-run-generic
// INTEL_CUSTOMIZATION
// UNSUPPORTED: x86_64-pc-linux-gnu
// TODO: Enable this test after required host runtime change lands on xmain
// end INTEL_CUSTOMIZATION

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

  int num_devices = omp_get_num_devices();

  // No target devices, just return
  if (num_devices == 0) {
    printf("PASS\n");
    return 0;
  }

  double sum = 999;
  double A = 311;

#pragma omp taskgroup task_reduction(+ : sum)
  {
#pragma omp target map(to : A) in_reduction(+ : sum) device(0) nowait
    { sum += A; }
  }

  printf("PASS\n");
  return EXIT_SUCCESS;
}

// CHECK: PASS
