// INTEL_COLLAB
// RUN: %clang_cc1 -ast-dump -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

class B {
public:
  B();
  void start();
  double* zoo;
  double* xoo;
  double* woo;
};
// CHECK: start 'void ()'
// CHECK-NOT: `-DeclRefExpr {{.*}} 'double *' lvalue OMPCapturedExpr {{.*}} 'woo' 'double *&'
void B::start()
{
  #pragma omp target map(tofrom: zoo[7:17]) map(from: xoo[1:6])
  zoo[2] = 7,xoo[2] = 8;
}
// end INTEL_COLLAB
