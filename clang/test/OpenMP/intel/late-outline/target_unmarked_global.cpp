// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline -std=c++17               \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -opaque-pointers -verify -triple spir64 -fopenmp           \
// RUN:  -fintel-compatibility -fopenmp-late-outline -std=c++17 -verify       \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device                           \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o %t.ll

extern "C" int printf(const char *, ...);

int *x;
int z;

int range(const int& size) { return (int)(size); }

struct square {
  //expected-note@+1 {{'count' declared here}}
  static constexpr int count = 2;

  void operator()(const int i) const {
    // expected-error@+1 {{variable 'count' used but not marked as declare target}}
    int end = range(count);
    printf("%d %i\n", end, i*i);
  }
};

void foo() {
  square a;
  #pragma omp target
  {
    a(4);
  }
}

void bar() {
  int i = 0;
  x[0] = 0; // Not an error, not device code.
  #pragma omp target map(to: x[:10])
  {
    x[i] = z + 1; // Not an error, covered by map clauses (implicit and explicit).
  }
}
// end INTEL_COLLAB
