// RUN: %clang_cl -MD -c -o %t %s
// RUN: %llvm_jitlink %t 2>&1 | FileCheck %s
// CHECK: Hello, world!

// INTEL_CUSTOMIZATION
// CMPLRLLVM-42778
// XFAIL: *
// end INTEL_CUSTOMIZATION

#include <iostream>
int main(int argc, char *argv[]) {
  std::cout << "Hello, world!" << std::endl;
  return 0;
}
