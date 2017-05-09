// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s -check-prefix=CHECK-REGION
//
// Checking on "regular" loops
//
int first1() noexcept;
int last1() noexcept;
int first2() noexcept;
int last2() noexcept;
int first3() noexcept;
int last3() noexcept;
void bar(int) noexcept;

// CHECK-LABEL: @_Z3fooPiPS_
void foo(int *arr1, int **arr2) {
  // CHECK-REGION: [[ARR2:%arr2.*]] = alloca i32**,
  // CHECK: [[ILCV:%i.*]] = alloca i32,
  // CHECK: [[JLCV:%j.*]] = alloca i32,
  // CHECK: [[ITER:%iter.*]] = alloca i32,
  // CHECK: [[KLCV:%k.*]] = alloca i32,
  int i,j;
  int iter;

// CHECK: directive(metadata !"DIR.OMP.SIMD")
// CHECK-NEXT: qual.opnd.i32(metadata !"QUAL.OMP.SAFELEN", i32 4)
// CHECK-NEXT: directive(metadata !"DIR.QUAL.LIST.END")
// CHECK: directive(metadata !"DIR.OMP.END.SIMD")
// CHECK: directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-REGION: [[TOKENVAL:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
// CHECK-REGION-SAME: "QUAL.OMP.SAFELEN"(i32 4)
// CHECK-REGION: call void @llvm.directive.region.exit(token [[TOKENVAL]]) [ "DIR.OMP.END.SIMD"() ]
  #pragma omp simd safelen(4)
  for (iter = first1(); iter < last1(); ++iter) {
    arr1[iter] = 42+iter;
  }


// CHECK: directive(metadata !"DIR.OMP.PARALLEL.LOOP")
// CHECK: directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
// CHECK-REGION: [[TOKENVAL2:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
// CHECK-REGION-SAME: "QUAL.OMP.SHARED"(i32*** [[ARR2]])
// CHECK-REGION: call void @llvm.directive.region.exit(token [[TOKENVAL2]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for collapse(2)
  for (i=first2(); i<last2(); ++i)
    for (j=first3(); j<last3(); ++j)
      arr2[i][j] = 42+i+j;
// CHECK: directive(metadata !"DIR.OMP.PARALLEL.LOOP")
// CHECK: directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
// CHECK-REGION: [[TOKENVAL3:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
// CHECK-REGION-SAME: "QUAL.OMP.PRIVATE"(i32* %k
// CHECK-REGION: call void @llvm.directive.region.exit(token [[TOKENVAL3]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for
  for (int k=0; k<10; k++) {
    bar(k);
  }
// CHECK: directive(metadata !"DIR.OMP.LOOP")
// CHECK: directive(metadata !"DIR.OMP.END.LOOP")
// CHECK-REGION: [[TOKENVAL4:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"()
// CHECK-REGION-SAME: "QUAL.OMP.PRIVATE"(i32* %k
// CHECK-REGION: call void @llvm.directive.region.exit(token [[TOKENVAL4]]) [ "DIR.OMP.END.LOOP"() ]
  #pragma omp for
  for (int k=0; k<10; k++) {
    bar(k);
  }

// CHECK: directive(metadata !"DIR.OMP.PARALLEL.LOOP")
// CHECK: directive(metadata !"DIR.OMP.SIMD")
// CHECK-REGION: [[TOKENVAL0:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
// CHECK-REGION-SAME: "QUAL.OMP.SHARED"(i32** %arr1
// CHECK-REGION: [[TOKENVAL1:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()
// CHECK-REGION-SAME: "QUAL.OMP.SAFELEN"(i32 4)
// CHECK-REGION: call void @llvm.directive.region.exit(token [[TOKENVAL1]]) [ "DIR.OMP.END.SIMD"() ]
// CHECK-REGION: call void @llvm.directive.region.exit(token [[TOKENVAL0]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for simd safelen(4)
  for (iter = first1(); iter < last1(); ++iter) {
    int pr = 4;
    arr1[iter] = 42+iter+pr;
  }
}
