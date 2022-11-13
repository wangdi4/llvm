// INTEL_COLLAB
// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
// RUN:   -std=c++14 -fexceptions -fcxx-exceptions -E %s                    \
// RUN:   -DTEN=10 -DA=a | FileCheck %s

int main() {
  int a;
  int b;
  int *p;

  // CHECK:#pragma ompx prefetch data(1:p[0:10]) if (a < b)
  #pragma ompx prefetch data(1:p[0:TEN]) if (A < b)
  return 0;
}
// end INTEL_COLLAB
