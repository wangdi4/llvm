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

struct ThreadData { int teams; int threads;};
// CHECK: foo
// CHECK-NOT: `-DeclRefExpr {{.*}} 'int' lvalue OMPCapturedExpr {{.*}} '.capture_expr.1'
// CHECK-NOT: `-DeclRefExpr {{.*}} 'int' lvalue OMPCapturedExpr {{.*}} '.capture_expr.1'
void foo(ThreadData* threads)
{
    #pragma omp target teams distribute parallel for num_teams(threads->teams) num_threads(threads->threads)
    for (int i = 0; i < 100; ++i)
    {
      threads->teams = 1;
    }
}

// end INTEL_COLLAB
