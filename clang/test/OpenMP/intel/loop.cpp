// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fopenmp-late-outline -triple x86_64-unknown-linux-gnu | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fexceptions -fopenmp -fintel-compatibility -fopenmp-late-outline -triple x86_64-unknown-linux-gnu | FileCheck %s
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
  // CHECK: [[ARR2:%arr2.*]] = alloca ptr,
  // CHECK: [[ILCV:%i.*]] = alloca i32,
  // CHECK: [[JLCV:%j.*]] = alloca i32,
  // CHECK: [[ITER:%iter.*]] = alloca i32,
  // CHECK: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // CHECK: [[OMP_IV10:%.omp.iv.*]] = alloca i64,
  // CHECK: [[OMP_IV72:%.omp.iv.*]] = alloca i32,
  // CHECK: [[KLCV:%k.*]] = alloca i32,
  // CHECK: [[OMP_IV88:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_IVAAA:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_IV105:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_IV138:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_IV178:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_IV218:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_UB218:%.omp.ub.*]] = alloca i32,
  // CHECK: [[OMP_IV236:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_UB236:%.omp.ub.*]] = alloca i32,
  // CHECK: [[OMP_IV255:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_LB255:%.omp.lb.*]] = alloca i32,
  // CHECK: [[OMP_UB255:%.omp.ub.*]] = alloca i32,
  // CHECK: [[OMP_IV288:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_UB288:%.omp.ub.*]] = alloca i32,
  int i,j;
  int iter;

// CHECK: [[TOKENVAL:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "DIR.OMP.SIMD"(),
// CHECK-SAME: "QUAL.OMP.SAFELEN"(i32 4)
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[OMP_IV]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr [[OMP_UB]]
// CHECK: call void @llvm.directive.region.exit(token [[TOKENVAL]])
// CHECK-SAME: [ "DIR.OMP.END.SIMD"() ]
  #pragma omp simd safelen(4)
  for (iter = first1(); iter < last1(); ++iter) {
    arr1[iter] = 42+iter;
  }

// CHECK: [[TOKENVAL2:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"(),
// CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[ARR2]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[OMP_IV10]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"
// CHECK: call void @llvm.directive.region.exit(token [[TOKENVAL2]])
// CHECK-SAME: [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for collapse(2)
  for (i=first2(); i<last2(); ++i)
    for (j=first3(); j<last3(); ++j)
      arr2[i][j] = 42+i+j;
// CHECK: [[TOKENVAL3:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[OMP_IV72]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"
// CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %k
// CHECK: call void @llvm.directive.region.exit(token [[TOKENVAL3]])
// CHECK-SAME: [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for
  for (int k=0; k<10; k++) {
    bar(k);
  }
// CHECK: [[TOKENVAL4:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "DIR.OMP.LOOP"()
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[OMP_IV88]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"
// CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %k
// CHECK: call void @llvm.directive.region.exit(token [[TOKENVAL4]])
// CHECK-SAME: [ "DIR.OMP.END.LOOP"() ]
  #pragma omp for
  for (int k=0; k<10; k++) {
    bar(k);
  }

// CHECK: [[TOKENVAL0:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "DIR.OMP.LOOP"()
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[OMP_IVAAA]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"
// CHECK: [[TOKENVAL1:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "DIR.OMP.SIMD"()
// CHECK-SAME: "QUAL.OMP.SAFELEN"(i32 4)
// CHECK: call void @llvm.directive.region.exit(token [[TOKENVAL1]])
// CHECK-SAME: [ "DIR.OMP.END.SIMD"() ]
// CHECK: call void @llvm.directive.region.exit(token [[TOKENVAL0]])
// CHECK-SAME: [ "DIR.OMP.END.LOOP"() ]
  #pragma omp for simd safelen(4)
  for (iter = first1(); iter < last1(); ++iter) {
    int pr = 4;
    arr1[iter] = 42+iter+pr;
  }

// CHECK: [[TOKENVAL0:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
// CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr %arr1
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[OMP_IV105]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"
// CHECK: [[TOKENVAL1:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "DIR.OMP.SIMD"()
// CHECK-SAME: "QUAL.OMP.SAFELEN"(i32 4)
// CHECK: call void @llvm.directive.region.exit(token [[TOKENVAL1]])
// CHECK-SAME: [ "DIR.OMP.END.SIMD"() ]
// CHECK: call void @llvm.directive.region.exit(token [[TOKENVAL0]])
// CHECK-SAME: [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for simd safelen(4)
  for (iter = first1(); iter < last1(); ++iter) {
    int pr = 4;
    arr1[iter] = 42+iter+pr;
  }
  int zii = 0;
// CHECK: DIR.OMP.TASKLOOP{{.*}}REDUCTION
 #pragma omp taskloop reduction(+:zii)
 for (iter = first1(); iter < last1(); ++iter) {
   int pr = 4;
   arr1[iter] = 42+iter+pr;
   zii += 42+iter+pr;
 }
// CHECK: DIR.OMP.TASKLOOP{{.*}}REDUCTION
// CHECK: DIR.OMP.SIMD
 #pragma omp taskloop simd reduction(+:zii)
 for (iter = first1(); iter < last1(); ++iter) {
   int pr = 4;
   arr1[iter] = 42+iter+pr;
   zii += 42+iter+pr;
 }

//CHECK: DIR.OMP.DISTRIBUTE.PARLOOP
//CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[OMP_IV218]]
//CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr [[OMP_UB218]]
//CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"{{.*}}%zz1
#pragma omp distribute parallel for
for (int i=0; i<16; ++i) {
  int zz1 = 1;
  bar(i+zz1);
}
//CHECK: DIR.OMP.END.DISTRIBUTE.PARLOOP

//CHECK: DIR.OMP.DISTRIBUTE.PARLOOP
//CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[OMP_IV236]]
//CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr [[OMP_UB236]]
//CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"{{.*}}%zz1
//CHECK: DIR.OMP.SIMD
#pragma omp distribute parallel for simd
for (int i=0; i<16; ++i) {
  int zz1 = 1;
  bar(i+zz1);
}
//CHECK: DIR.OMP.END.SIMD
//CHECK: DIR.OMP.END.DISTRIBUTE.PARLOOP

// CHECK: DIR.OMP.PARALLEL.LOOP
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[OMP_IV255]]
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[OMP_LB255]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr [[OMP_UB255]]
  #pragma omp parallel for
  for (iter = first1(); iter < last1(); ++iter) {
    int pr = 4;
    arr1[iter] = 42+iter+pr;
  }

// CHECK: [[TV288:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "DIR.OMP.SIMD"(),
// CHECK-SAME: "QUAL.OMP.SAFELEN"(i32 4)
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[OMP_IV288]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr [[OMP_UB288]]
// CHECK: call void @llvm.directive.region.exit(token [[TV288]])
// CHECK-SAME: [ "DIR.OMP.END.SIMD"() ]
  #pragma omp simd safelen(4)
  for (iter = 4; iter < 14; ++iter) {
    arr1[iter] = 42+iter;
  }

 #pragma omp parallel for
 #pragma unroll(4)
 for (iter = first1(); iter < last1(); ++iter) { }
 // CHECK: br {{.*}}label {{.*}}, !llvm.loop ![[LOOP_1:.*]]

 #pragma omp parallel for
 #pragma unroll
 for (iter = first1(); iter < last1(); ++iter) { }
 // CHECK: br {{.*}}label {{.*}}, !llvm.loop ![[LOOP_2:.*]]
}

// CHECK: doacross_test
const int M = 5;
const int N = 4;
void doacross_test(int (*v_ptr)[5][4])
{
  // CHECK: [[VARI:%i.*]] = alloca i32,
  // CHECK: [[VARJ:%j.*]] = alloca i32,
  // CHECK: [[OMPIV:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMPLB:%.omp.lb.*]] = alloca i32,
  // CHECK: [[OMPUB:%.omp.ub.*]] = alloca i32,
  int i, j;
  // CHECK: region.entry{{.*}}OMP.PARALLEL.LOOP
  // CHECK-SAME: ORDERED"(i32 2, i32 4, i32 2)
  #pragma omp parallel for ordered (2) private (j)
  for (i = 1; i < M; i++) {
    for (j = 2; j < N; j++) {
  // CHECK: [[DAL1:%[0-9]+]] = load{{.*}}[[OMPLB]]
  // CHECK: store i32 [[DAL1]], ptr [[OMPIV]]
  // CHECK: [[DAL2:%[0-9]+]] = load{{.*}}[[OMPIV]]
  // CHECK: [[DAL3:%[0-9]+]] = load{{.*}}[[OMPUB]]
  // CHECK: icmp sle i32 [[DAL2]], [[DAL3]]
  // CHECK: [[DAL4:%[0-9]+]] = load{{.*}}[[OMPIV]]
  // CHECK: store i32{{.*}}[[VARI]]
  // CHECK: store i32{{.*}}[[VARJ]]

  // CHECK: [[N6:%[0-9]+]] = load{{.*}}[[VARI]]
  // CHECK: [[S:%sub[0-9]*]] = sub nsw i32 [[N6]], 1
  // CHECK: [[S2:%sub[0-9]+]] = sub nsw i32 [[S]], 1
  // CHECK: [[D:%div[0-9]*]] = sdiv i32 [[S2]], 1

  // CHECK: [[N7:%[0-9]+]] = load{{.*}}[[VARJ]]
  // CHECK: [[S3:%sub[0-9]*]] = sub nsw i32 [[N7]], 1
  // CHECK: [[S4:%sub[0-9]+]] = sub nsw i32 [[S3]], 2
  // CHECK: [[D5:%div[0-9]+]] = sdiv i32 [[S4]], 1

  // CHECK: [[N8:%[0-9]+]] = load{{.*}}[[VARI]]
  // CHECK: [[S6:%sub[0-9]*]] = sub nsw i32 [[N8]], 1
  // CHECK: [[D7:%div[0-9]+]] = sdiv i32 [[S6]], 1

  // CHECK: [[N9:%[0-9]+]] = load{{.*}}[[VARJ]]
  // CHECK: [[S8:%sub[0-9]*]] = sub nsw i32 [[N9]], 2
  // CHECK: [[S9:%sub[0-9]+]] = sub nsw i32 [[S8]], 2
  // CHECK: [[D10:%div[0-9]+]] = sdiv i32 [[S9]], 1

  // CHECK: region.entry{{.*}}DIR.OMP.ORDERED
  // CHECK-SAME: "QUAL.OMP.DEPEND.SINK"(i32 [[D]], i32 [[D5]])
  // CHECK-SAME: "QUAL.OMP.DEPEND.SINK"(i32 [[D7]], i32 [[D10]])
  // CHECK: region.exit{{.*}}DIR.OMP.END.ORDERED
      #pragma omp ordered depend(sink: i-1, j-1) depend(sink: i, j-2)
      (*v_ptr) [i][j] = (*v_ptr) [i-1][j - 1] + (*v_ptr) [i][j-2];

  //CHECK: call {{.*}}bar
  bar(0);
  //CHECK: [[N22:%[0-9]+]] = load i32, ptr [[VARI]], align 4
  //CHECK: [[SUB25:%sub[0-9]+]] = sub nsw i32 [[N22]], 1
  //CHECK: [[DIV26:%div[0-9]+]] = sdiv i32 [[SUB25]], 1
  //CHECK: [[N23:%[0-9]+]] = load i32, ptr [[VARJ]], align 4
  //CHECK: [[SUB27:%sub[0-9]+]] = sub nsw i32 [[N23]], 2
  //CHECK: [[DIV28:%div[0-9]+]] = sdiv i32 [[SUB27]], 1

  // CHECK: region.entry{{.*}}DIR.OMP.ORDERED
  // CHECK-SAME: "QUAL.OMP.DEPEND.SOURCE"
  // CHECK-SAME: (i32 [[DIV26]], i32 [[DIV28]])
  // CHECK: region.exit{{.*}}DIR.OMP.END.ORDERED
      #pragma omp ordered depend(source)
    }
  }
  // CHECK: region.exit{{.*}}OMP.END.PARALLEL.LOOP
}

// CHECK: doacross_test_two
void doacross_test_two(int (*v_ptr)[5][4])
{
// CHECK: [[I:%i.*]] = alloca i32,
// CHECK: [[J:%j.*]] = alloca i32,
  int i, j;
  // CHECK: region.entry{{.*}}OMP.PARALLEL.LOOP
  // CHECK-SAME: ORDERED"(i32 2, i32 4, i32 2)
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[I]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  #pragma omp parallel for ordered (2)
  for (i = 1; i < M; i++) {
    for (j = 2; j < N; j++) {
    }
  }
}

// CHECK: !llvm.ident
// CHECK: ![[LOOP_1]] = distinct !{![[LOOP_1]], ![[UNROLL_4:.*]]}
// CHECK: ![[UNROLL_4]] = !{!"llvm.loop.unroll.count", i32 4}
// CHECK: ![[LOOP_2]] = distinct !{![[LOOP_2]], ![[UNROLL_ENABLE:.*]]}
// CHECK: ![[UNROLL_ENABLE]] = !{!"llvm.loop.unroll.enable"}
