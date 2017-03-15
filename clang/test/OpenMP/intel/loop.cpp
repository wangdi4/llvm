// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s
int first1() noexcept;
int last1() noexcept;
int first2() noexcept;
int last2() noexcept;
int first3() noexcept;
int last3() noexcept;
void bar(int) noexcept;

// CHECK-LABEL: @_Z3fooPiPS_
void foo(int *arr1, int **arr2) {
  // CHECK: [[ILCV:%i.*]] = alloca i32,
  // CHECK: [[JLCV:%j.*]] = alloca i32,
  // CHECK: [[KLCV:%k.*]] = alloca i32,
  int i,j;
  int iter;

// Ensure the PHI loop structure is there
// CHECK: directive(metadata !"DIR.OMP.SIMD")
// CHECK-NEXT: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* %.omp.iv
// CHECK-NEXT: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* %iter
// CHECK-NEXT: qual.opnd.i32(metadata !"QUAL.OMP.SAFELEN", i32 4)
// CHECK-NEXT: directive(metadata !"DIR.QUAL.LIST.END")
// CHECK: [[PHIVAL:%[0-9]+]] = phi i32 [ 0, {{%.+}} ], [ [[INCVAL:%[0-9]+]], {{.+}} ]
// CHECK-NEXT: store i32 [[PHIVAL]], i32* %.omp.iv
// CHECK: [[INCVAL]] = add i32 [[PHIVAL]], 1
  #pragma omp simd safelen(4)
  for (iter = first1(); iter < last1(); ++iter) {
    arr1[iter] = 42+iter;
  }

// Ensure the LCVs are getting updates
// CHECK: directive(metadata !"DIR.OMP.PARALLEL.LOOP")
// CHECK: %{{[[0-9]+}} = load {{.*}} [[IV:%.omp.iv[0-9]*]]
// CHECK: store {{.*}} [[ILCV]],
// CHECK: %{{[[0-9]+}} = load {{.*}} [[IV:%.omp.iv[0-9]*]]
// CHECK: store {{.*}} [[JLCV]],
// CHECK: directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  #pragma omp parallel for collapse(2)
  for (i=first2(); i<last2(); ++i)
    for (j=first3(); j<last3(); ++j)
      arr2[i][j] = 42+i+j;

// Declared form of lcv
// CHECK: directive(metadata !"DIR.OMP.PARALLEL.LOOP")
// CHECK: %{{[[0-9]+}} = load {{.*}} [[IV2:%.omp.iv[0-9]*]]
// CHECK: store {{.*}} [[KLCV]],
// CHECK: directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  #pragma omp parallel for
  for (int k=0; k<10; k++) {
    bar(k);
  }
}
