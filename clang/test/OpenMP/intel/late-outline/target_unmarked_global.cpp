// INTEL_COLLAB
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline -std=c++17               \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -verify -triple spir64 -fopenmp           \
// RUN:  -fintel-compatibility -fopenmp-late-outline -std=c++17 -verify       \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device                           \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s

// expected-no-diagnostics

// CHECK: @_ZN6square5countE = linkonce_odr target_declare addrspace(1) constant i32 2, comdat
// CHECK: @x = external addrspace(1) global ptr addrspace(4)
// CHECK: @_ZN3foo1nE.const = private target_declare unnamed_addr addrspace(1) constant [2 x i32] [i32 0, i32 1]
// CHECK: @_ZL8n_global.const = private target_declare unnamed_addr addrspace(1) constant [2 x i32] [i32 1, i32 2]
// CHECK: @_ZL5array.const = private target_declare unnamed_addr addrspace(1) constant [2 x float] [float 0.000000e+00, float 1.000000e+00]
// CHECK: @_ZN1A1aE = linkonce_odr target_declare addrspace(1) constant [2 x float] [float 0.000000e+00, float 1.000000e+00], comdat
// CHECK-NOT @_ZL8not_used.const = private target_declare
int *x;
int z;

int range(const int& size) { return (int)(size); }

struct square {
  static constexpr int count = 2;

  void operator()(const int i) const {
    int end = range(count);
  }
};

// CHECK-LABEL: @_Z3foov(
void foo() {
  square a;
// CHECK: "DIR.OMP.TARGET"
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %a.ascas
// CHECK "DIR.OMP.END.TARGET"
  #pragma omp target
  {
    a(4);
  }
}

// CHECK-LABEL: @_Z3barv(
void bar() {
  int i = 0;
  x[0] = 0; // Not an error, not device code.
// CHECK:  "DIR.OMP.TARGET"
// CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @x to ptr addrspace(4)), ptr addrspace(4) %arrayidx1, i64 40, i64 49
// CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target map(to: x[:10])
  {
    x[i] = z + 1; // Not an error, covered by map clauses (implicit and explicit).
  }
}

float f;
// CHECK-LABEL: @_Z4calcv(
void calc()
{
  f = 0.0;

  // CHECK: "DIR.OMP.TARGET"
  // CHECK-SEAM: "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @f to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @f to ptr addrspace(4))
  // CHECK: "DIR.OMP.END.TARGET"
  // No error here. This use of 'f' is not in a target region.
  #pragma omp target teams distribute parallel for simd reduction(+:f)
  for (int i=1; i<=8; i++) {}
}

struct foo {
  static constexpr int n[] = {0,1};
};

static constexpr int n_global[] = {1,2};

constexpr float array[] = { 0.0f, 1.0f };
constexpr float not_used[] = { 0.0f, 1.0f };
struct A {
  static constexpr float a[] = { 0.0f, 1.0f };
  inline const float& get0() {
    return a[1];
  }
};

void moo()
{
  A avar;
  float f1 = avar.get0();
}

void other ()
{
  float f = not_used[0];
}

// CHECk-LABEL: @main(
int main()
{
  int m = -1;
// CHECK: "DIR.OMP.TARGET"()
// CHECK-SAME: "QUAL.OMP.MAP.FROM"(ptr addrspace(4) %m.ascast
// CHECK: "DIR.OMP.END.TARGET"()
  #pragma omp target map(from:m)
  m = foo::n[0] + n_global[1];
// CHECK: "DIR.OMP.TARGET"()
// CHECK-NOT: "QUAL.OMP.MAP.FROM"
// CHECK: "DIR.OMP.END.TARGET"()
  #pragma omp target
  float f = array[0];
// CHECK: "DIR.OMP.TARGET"()
// CHECK-NOT: "QUAL.OMP.MAP.FROM"
// CHECK: "DIR.OMP.END.TARGET"()
  #pragma omp target
  moo();
  other();
  calc();
}
// end INTEL_COLLAB
