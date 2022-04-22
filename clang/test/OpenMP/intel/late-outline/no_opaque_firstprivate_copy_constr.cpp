// INTEL_COLLAB
// Verify copy constructor with cleanups compiles with late outlining.
//
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

struct St {
int a, b;
St() : a(0), b(0) {}
St(const St &st) : a(st.a + st.b), b(0) {}
~St() {}
};

struct S {
int f;
S() : f(1212) {}
S(const S &s, St t = St()) : f(s.f + t.a) {}
~S() {}
};

int tmain() {
  S test;
// CHECK: "DIR.OMP.PARALLEL"
// CHECK: "DIR.OMP.LOOP"()
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%struct.S* %test, void (%struct.S*, %struct.S*)*
// CHECK-SAME: @_{{.*}}omp.copy_constr,
// CHECK-SAME: void (%struct.S*)*
// CHECK-SAME: @_{{.*}}omp.destr)
// CHECK: "DIR.OMP.END.LOOP"
// CHECK: "DIR.OMP.END.PARALLEL"
#pragma omp parallel
#pragma omp for firstprivate(test)
  for (int i = 0; i < 1; ++i) {
  }
  return int();
}

int main() {
  return tmain();
}
// end INTEL_COLLAB
