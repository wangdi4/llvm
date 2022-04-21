// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
int f(int);
int aa;
int *a;
unsigned jg;
// CHECK-LABEL @foo
void foo(int m1, int n1, int m2, int **b, int n2 )
{
  unsigned i,j,k;
  i=0;
  // CHECK: DIR.OMP.PARALLEL.LOOP
  // CHECK: QUAL.OMP.LASTPRIVATE{{.*}}ptr %j
  // CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for lastprivate(j)
  for (j=m2+1;j>n2;j-=a[m2]) {
    #pragma omp critical
    b[i][j] = b[i][j-1]+i+j;
  }

  // CHECK: DIR.OMP.SIMD
  // CHECK: QUAL.OMP.LINEAR:IV{{.*}}ptr %j
  // CHECK: DIR.OMP.END.SIMD
  #pragma omp simd
  for (j=m2+1;j>n2;j-=a[m2]) {
    b[i][j] = b[i][j-1]+i+j;
  }

  // CHECK: DIR.OMP.SIMD
  // CHECK: QUAL.OMP.LINEAR:IV{{.*}}ptr %j
  // CHECK: DIR.OMP.END.SIMD
  #pragma omp simd
  for (unsigned j=m2+1;j>n2;j-=a[m2]) {
    b[i][j] = b[i][j-1]+i+j;
  }

  // CHECK: DIR.OMP.SIMD
  // CHECK: QUAL.OMP.LINEAR:IV{{.*}}ptr @jg
  // CHECK: DIR.OMP.END.SIMD
  #pragma omp simd
  for (jg=m2+1;jg>n2;jg-=a[m2]) {
    b[i][jg] = b[i][jg-1]+i+jg;
  }
  {
    int ii,kk;
    // CHECK: DIR.OMP.SIMD
    // CHECK: QUAL.OMP.LASTPRIVATE{{.*}}ptr %ii
    // CHECK-NOT: QUAL.OMP{{.*}}ptr %jj
    // CHECK: QUAL.OMP.LASTPRIVATE{{.*}}ptr %kk
    // CHECK: DIR.OMP.END.SIMD
    #pragma omp simd collapse(3)
    for (ii=0;ii<10;++ii)
    for (int jj=0;jj<10;++jj)
    for (kk=0;kk<10;++kk) {}
  }
}
// end INTEL_COLLAB
