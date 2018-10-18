// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fintel-compatibility \
// RUN:  -fintel-openmp-region -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck %s

int f(int);
int aa;
int *a;
// CHECK-LABEL @foo
void foo(int m1, int n1, int m2, int **b, int n2 )
{
  unsigned i,j,k;
  i=0;
  // CHECK: DIR.OMP.PARALLEL.LOOP
  // CHECK: QUAL.OMP.LASTPRIVATE{{.*}}i32* %j
  // CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for lastprivate(j)
    for (j=m2+1;j>n2;j-=a[m2]) {
      #pragma omp critical
      b[i][j] = b[i][j-1]+i+j;
    }
}
