// JIRA: CMPLRLLVM-48308
// Issue a warning when there is no symbolizer in PATH

// REQUIRES: asan
// RUN: %clangxx -O0 -g %s -o %t
// RUN: not env -i %t 2>&1 | FileCheck %s

// CHECK: WARNING: No symbolizer is found
int main(int argc, char **argv) {
  int *array = new int[100];
  delete[] array;
  return array[argc]; // BOOM
}
