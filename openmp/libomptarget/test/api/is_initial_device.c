// RUN: %libomptarget-compile-run-and-check-x86_64-pc-linux-gnu
// RUN: %libomptarget-compile-x86_64-pc-linux-gnu -DUNUSED -Wall -Werror

// INTEL_CUSTOMIZATION
// Disable use of variants for the second RUN line until proper fix is done in
// omp.h.
#ifndef UNUSED
// end INTEL_CUSTOMIZATION
#include <omp.h>
// INTEL_CUSTOMIZATION
#endif
// end INTEL_CUSTOMIZATION
#include <stdio.h>

int main() {
  int errors = 0;
#ifdef UNUSED
// Test if it is OK to leave the variants unused in the header
#else // UNUSED
  int host = omp_is_initial_device();
  int device = 1;
#pragma omp target map(tofrom : device)
  { device = omp_is_initial_device(); }
  if (!host) {
    printf("omp_is_initial_device() returned false on host\n");
    errors++;
  }
  if (device) {
    printf("omp_is_initial_device() returned true on device\n");
    errors++;
  }
#endif // UNUSED

  // CHECK: PASS
  printf("%s\n", errors ? "FAIL" : "PASS");

  return errors;
}
