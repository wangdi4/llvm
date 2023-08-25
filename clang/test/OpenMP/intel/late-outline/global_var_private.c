// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

void bar(int);

int u;

// CHECK-LABEL: @main
int main () {
  int     k;
//CHECK: region.entry{{.*}}OMP.SIMD{{.*}}PRIVATE:TYPED"(ptr @u
  #pragma omp simd private(u)
  for ( k = 0; k < 1; k ++ ) { u = 2; }
//CHECK: region.entry{{.*}}OMP.LOOP{{.*}}PRIVATE:TYPED"(ptr @u
  #pragma omp for private(u)
  for ( k = 0; k < 1; k ++ ) { u = 2; }
//CHECK: region.entry{{.*}}OMP.PARALLEL.LOOP{{.*}}PRIVATE:TYPED"(ptr @u
  #pragma omp parallel for private(u)
  for ( k = 0; k < 1; k ++ ) { u = 2; }
//CHECK: region.entry{{.*}}OMP.PARALLEL.LOOP{{.*}}PRIVATE:TYPED"(ptr @u
//CHECK: region.entry{{.*}}OMP.SIMD{{.*}}PRIVATE:TYPED"(ptr @u
  #pragma omp parallel for simd private(u)
  for ( k = 0; k < 1; k ++ ) { u = 2; }
//CHECK: region.entry{{.*}}OMP.TASKLOOP{{.*}}PRIVATE:TYPED"(ptr @u
  #pragma omp taskloop private(u)
  for ( k = 0; k < 1; k ++ ) { u = 2; }
//CHECK: region.entry{{.*}}OMP.TASKLOOP{{.*}}PRIVATE:TYPED"(ptr @u
//CHECK: region.entry{{.*}}OMP.SIMD{{.*}}PRIVATE:TYPED"(ptr @u
  #pragma omp taskloop simd private(u)
  for ( k = 0; k < 1; k ++ ) { u = 2; }
//CHECK: region.entry{{.*}}OMP.PARALLEL{{.*}}PRIVATE:TYPED"(ptr @u
  #pragma omp parallel private(u)
  for ( k = 0; k < 1; k ++ ) { u = 2; }

//CHECK: region.entry{{.*}}OMP.PARALLEL
  #pragma omp parallel
  {
//CHECK: region.entry{{.*}}OMP.SECTIONS{{.*}}PRIVATE:TYPED"(ptr @u
    #pragma omp sections private(u)
    {
      #pragma omp section
      {
        bar(1+u);
      }
      #pragma omp section
      {
        bar(2+u);
      }
    }
//CHECK: region.entry{{.*}}OMP.SINGLE{{.*}}PRIVATE:TYPED"(ptr @u
    #pragma omp single private(u)
    {
        bar(3+u);
    }
  }

//CHECK: region.entry{{.*}}OMP.TASK{{.*}}PRIVATE:TYPED"(ptr @u
  #pragma omp task private(u)
  {
    bar(4+u);
  }
}
// end INTEL_COLLAB
