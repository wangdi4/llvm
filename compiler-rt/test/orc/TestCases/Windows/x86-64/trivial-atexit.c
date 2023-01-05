// RUN: %clang_cl -MD -c -o %t %s
// RUN: %llvm_jitlink %t 2>&1 | FileCheck %s

// INTEL_CUSTOMIZATION
// CMPLRLLVM-42778
// XFAIL: *
// end INTEL_CUSTOMIZATION

// CHECK: Entering main
// CHECK-NEXT: Meow

#include <stdio.h>
#include <stdlib.h>

void meow() {
  printf("Meow\n");
  fflush(stdout);
}

int main(int argc, char *argv[]) {
  atexit(meow);
  printf("Entering main\n");
  return 0;
}
