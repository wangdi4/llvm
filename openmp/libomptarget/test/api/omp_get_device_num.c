// RUN: %libomptarget-compile-run-and-check-generic
// INTEL_CUSTOMIZATION
// Disable this test until build system starts picking up the latest host
// runtime containing a correct fix.
// UNSUPPORTED: x86_64-pc-linux-gnu, x86_64-pc-linux-gnu-LTO
// end INTEL_CUSTOMIZATION

#include <stdio.h>
#include <omp.h>

int test_omp_get_device_num()
{
  /* checks that omp_get_device_num() == omp_get_num_devices() in the host */
  int device_num = omp_get_device_num();
  printf("device_num = %d\n", device_num);

  #pragma omp target
  {}

  return (device_num == omp_get_num_devices());
}

int main()
{
  int i;
  int failed=0;

  if (!test_omp_get_device_num()) {
    failed++;
  }
  if (failed)
    printf("FAIL\n");
  else
    printf("PASS\n");
  return failed;
}

// CHECK: PASS
