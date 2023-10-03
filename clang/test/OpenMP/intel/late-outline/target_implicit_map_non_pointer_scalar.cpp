// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline -x c++ \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

class A
{
 public:
  double scalar=1.23;
// CHECK-LABEL: dowork
  void dowork (void)
  {
// CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
    #pragma omp target firstprivate(scalar)
    {
      scalar = 1.0;
    }
  }
};


void bar() {
  A a;
  a.dowork();
}

// end INTEL_COLLAB
