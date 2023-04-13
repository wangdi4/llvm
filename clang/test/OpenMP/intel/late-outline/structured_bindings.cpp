// INTEL_COLLAB
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp  \
// RUN:  -fopenmp-late-outline -fopenmp-version=51 -fsyntax-only %s
//
//expected-no-diagnostics

template <typename... T>
void b(T... c) {
  int a[2] = {1,2};
  auto&& [x,y] = a;
  (void)x;
}

// end INTEL_COLLAB
