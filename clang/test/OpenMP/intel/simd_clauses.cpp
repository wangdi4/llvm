// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s

// CHECK-LABEL: @_Z3foov
void foo()
{
  // CHECK: [[I_ADDR:%.+]] = alloca i32,
  // CHECK: [[J_ADDR:%.+]] = alloca i32,
  // CHECK: [[Y_ADDR:%.+]] = alloca i32*,
  // CHECK: [[Z_ADDR:%.+]] = alloca i32*,
  // CHECK: [[X_ADDR:%.+]] = alloca i32*,
  // CHECK: [[Q_ADDR:%.+]] = alloca i32*,
  int i,j;
  int *y,*z,*x, *q;

  // CHECK: directive(metadata !"DIR.OMP.SIMD")
  // CHECK: qual.opnd.i32(metadata !"QUAL.OMP.SAFELEN", i32 4)
  // CHECK: qual.opnd.i32(metadata !"QUAL.OMP.SIMDLEN", i32 4)
  // CHECK: qual.opnd.i32(metadata !"QUAL.OMP.COLLAPSE", i32 2)
  // CHECK: qual.opndlist(metadata !"QUAL.OMP.ALIGNED", i32** [[Y_ADDR]], i32** [[Z_ADDR]], i32 8)
  // CHECK: opndlist(metadata !"QUAL.OMP.ALIGNED", i32** [[X_ADDR]], i32 4)
  // CHECK: opndlist(metadata !"QUAL.OMP.ALIGNED", i32** [[Q_ADDR]], i32 0)
  // CHECK: directive(metadata !"DIR.QUAL.LIST.END")
  #pragma omp simd safelen(4) simdlen(4) collapse(2) \
            aligned(y,z:8) aligned(x:4) aligned(q)
  for (i=0;i<10;++i)
  for (j=0;j<10;++j) {}
}
