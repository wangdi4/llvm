// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s -check-prefix=CHECK-REGION
// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fexceptions -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s -check-prefix=CHECK-REGION
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
// CHECK-NEXT: qual.opndlist(metadata !"QUAL.OMP.NORMALIZED.IV"
// CHECK-NEXT: directive(metadata !"DIR.QUAL.LIST.END")
// CHECK: directive(metadata !"DIR.OMP.END.SIMD")
// CHECK: directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-REGION: [[TOKENVAL:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
// CHECK-REGION-SAME: "QUAL.OMP.SAFELEN"(i32 4)
// CHECK-REGION-SAME: "QUAL.OMP.NORMALIZED.IV"
// CHECK-REGION: call void @llvm.directive.region.exit(token [[TOKENVAL]]) [ "DIR.OMP.END.SIMD"() ]
  #pragma omp simd safelen(4)
  for (iter = first1(); iter < last1(); ++iter) {
    arr1[iter] = 42+iter;
  }


// CHECK: directive(metadata !"DIR.OMP.PARALLEL.LOOP")
// CHECK: directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
// CHECK-REGION: [[TOKENVAL2:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
// CHECK-REGION-SAME: "QUAL.OMP.NORMALIZED.IV"
// CHECK-REGION-SAME: "QUAL.OMP.SHARED"(i32*** [[ARR2]])
// CHECK-REGION: call void @llvm.directive.region.exit(token [[TOKENVAL2]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for collapse(2)
  for (i=first2(); i<last2(); ++i)
    for (j=first3(); j<last3(); ++j)
      arr2[i][j] = 42+i+j;
// CHECK: directive(metadata !"DIR.OMP.PARALLEL.LOOP")
// CHECK: directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
// CHECK-REGION: [[TOKENVAL3:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
// CHECK-REGION-SAME: "QUAL.OMP.NORMALIZED.IV"
// CHECK-REGION-SAME: "QUAL.OMP.PRIVATE"(i32* %k
// CHECK-REGION: call void @llvm.directive.region.exit(token [[TOKENVAL3]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for
  for (int k=0; k<10; k++) {
    bar(k);
  }
// CHECK: directive(metadata !"DIR.OMP.LOOP")
// CHECK: directive(metadata !"DIR.OMP.END.LOOP")
// CHECK-REGION: [[TOKENVAL4:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"()
// CHECK-REGION-SAME: "QUAL.OMP.NORMALIZED.IV"
// CHECK-REGION-SAME: "QUAL.OMP.PRIVATE"(i32* %k
// CHECK-REGION: call void @llvm.directive.region.exit(token [[TOKENVAL4]]) [ "DIR.OMP.END.LOOP"() ]
  #pragma omp for
  for (int k=0; k<10; k++) {
    bar(k);
  }

// CHECK: directive(metadata !"DIR.OMP.PARALLEL.LOOP")
// CHECK: directive(metadata !"DIR.OMP.SIMD")
// CHECK-REGION: [[TOKENVAL0:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
// CHECK-REGION-SAME: "QUAL.OMP.NORMALIZED.IV"
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
  int zii = 0;
// CHECK-REGION: DIR.OMP.TASKLOOP{{.*}}REDUCTION
 #pragma omp taskloop reduction(+:zii)
 for (iter = first1(); iter < last1(); ++iter) {
   int pr = 4;
   arr1[iter] = 42+iter+pr;
   zii += 42+iter+pr;
 }
// CHECK-REGION: DIR.OMP.TASKLOOP{{.*}}REDUCTION
// CHECK-REGION: DIR.OMP.SIMD
 #pragma omp taskloop simd reduction(+:zii)
 for (iter = first1(); iter < last1(); ++iter) {
   int pr = 4;
   arr1[iter] = 42+iter+pr;
   zii += 42+iter+pr;
 }
// CHECK-REGION: DIR.OMP.PARALLEL.LOOP{{.*}}FIRSTPRIVATE{{.*}}%.omp.lb
  #pragma omp parallel for
  for (iter = first1(); iter < last1(); ++iter) {
    int pr = 4;
    arr1[iter] = 42+iter+pr;
  }
// CHECK-REGION: DIR.OMP.PARALLEL.LOOP{{.*}}FIRSTPRIVATE{{.*}}%.omp.ub
  #pragma omp parallel for
  for (iter = first1(); iter < last1(); ++iter) {
    int pr = 4;
    arr1[iter] = 42+iter+pr;
  }
 #pragma omp parallel for
 #pragma unroll(4)
 for (iter = first1(); iter < last1(); ++iter) { }
 // CHECK-REGION: br label {{.*}}, !llvm.loop ![[LOOP_1:.*]]

 #pragma omp parallel for
 #pragma unroll
 for (iter = first1(); iter < last1(); ++iter) { }
 // CHECK-REGION: br label {{.*}}, !llvm.loop ![[LOOP_2:.*]]
}

// CHECK-REGION: doacross_test
const int M = 5;
const int N = 4;
void doacross_test(int (*v_ptr)[5][4])
{
  // CHECK-REGION: [[VARI:%i.*]] = alloca i32,
  // CHECK-REGION: [[VARJ:%j.*]] = alloca i32,
  // CHECK-REGION: [[OMPIV:%.omp.iv.*]] = alloca i32,
  // CHECK-REGION: [[OMPLB:%.omp.lb.*]] = alloca i32,
  // CHECK-REGION: [[OMPUB:%.omp.ub.*]] = alloca i32,
  int i, j;
  // CHECK-REGION: region.entry{{.*}}OMP.PARALLEL.LOOP{{.*}}ORDERED"(i32 2)
  #pragma omp parallel for ordered (2) private (j)
  for (i = 1; i < M; i++) {
    for (j = 2; j < N; j++) {
  // CHECK-REGION: [[DAL1:%[0-9]+]] = load{{.*}}[[OMPLB]]
  // CHECK_REGION: store i32 [[DAL1]], i32* [[OMPIV]]
  // CHECK-REGION: [[DAL2:%[0-9]+]] = load{{.*}}[[OMPIV]]
  // CHECK-REGION: [[DAL3:%[0-9]+]] = load{{.*}}[[OMPUB]]
  // CHECK_REGION: icmp sle i32 [[DAL2]], [[DAL3]]
  // CHECK-REGION: [[DAL4:%[0-9]+]] = load{{.*}}[[OMPIV]]
  // CHECK_REGION: store i32{{.*}}[[VARI]]
  // CHECK-REGION: [[DAL5:%[0-9]+]] = load{{.*}}[[OMPIV]]
  // CHECK_REGION: store i32{{.*}}[[VARJ]]
  // CHECK-REGION: [[DAL6:%[0-9]+]] = load{{.*}}[[OMPIV]]
  // CHECK-REGION: [[DAS:%sub[0-9]*]] = sub nsw i32 [[DAL6]], 1
  // CHECK-REGION: [[DAS4:%sub[0-9]+]] = sub nsw i32 [[DAS]], 2
  // CHECK-REGION: [[DAL7:%[0-9]+]] = load{{.*}}[[OMPIV]]
  // CHECK-REGION: [[DAS5:%sub[0-9]+]] = sub nsw i32 [[DAL7]], 2
  // CHECK-REGION: region.entry{{.*}}DIR.OMP.ORDERED
  // CHECK-REGION-SAME: "QUAL.OMP.DEPEND.SINK"(i32 [[DAS4]])
  // CHECK-REGION-SAME: "QUAL.OMP.DEPEND.SINK"(i32 [[DAS5]])
  // CHECK-REGION: region.exit{{.*}}DIR.OMP.END.ORDERED
      #pragma omp ordered depend(sink: i-1, j-1) depend(sink: i, j-2)
      (*v_ptr) [i][j] = (*v_ptr) [i-1][j - 1] + (*v_ptr) [i][j-2];

  // CHECK-REGION: region.entry{{.*}}DIR.OMP.ORDERED
  // CHECK-REGION-SAME: "QUAL.OMP.DEPEND.SOURCE"
  // CHECK-REGION: region.exit{{.*}}DIR.OMP.END.ORDERED
      #pragma omp ordered depend(source)
    }
  }
  // CHECK-REGION: region.exit{{.*}}OMP.END.PARALLEL.LOOP
}

// CHECK-REGION: !llvm.ident
// CHECK-REGION: ![[LOOP_1]] = distinct !{![[LOOP_1]], ![[UNROLL_4:.*]]}
// CHECK-REGION: ![[UNROLL_4]] = !{!"llvm.loop.unroll.count", i32 4}
// CHECK-REGION: ![[LOOP_2]] = distinct !{![[LOOP_2]], ![[UNROLL_ENABLE:.*]]}
// CHECK-REGION: ![[UNROLL_ENABLE]] = !{!"llvm.loop.unroll.enable"}
