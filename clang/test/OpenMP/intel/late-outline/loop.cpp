// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -std=c++14 -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -std=c++14 -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -fopenmp-new-depend-ir -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck %s

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
  // CHECK: [[ARR1:%arr1.*]] = alloca ptr,
  // CHECK: [[ARR2:%arr2.*]] = alloca ptr,
  // CHECK: [[I:%i.*]] = alloca i32,
  // CHECK: [[J:%j.*]] = alloca i32,
  // CHECK: [[ITER:%iter.*]] = alloca i32,
  // CHECK: [[IV:%.omp.iv.*]] = alloca i32,
  // CHECK: [[IV11:%.omp.iv.*]] = alloca i64,
  // CHECK: [[IV72:%.omp.iv.*]] = alloca i32,
  // CHECK: [[K:%k.*]] = alloca i32,
  // CHECK: [[IV88:%.omp.iv.*]] = alloca i32,
  // CHECK: [[K97:%k.*]] = alloca i32,
  // CHECK: [[IV105:%.omp.iv.*]] = alloca i32,
  // CHECK: [[IV138:%.omp.iv.*]] = alloca i32,
  // CHECK: [[IV178:%.omp.iv.*]] = alloca i32,
  int i,j;
  int iter;

// CHECK: [[T4:%[0-9]+]] = call token {{.*}}region.entry() [ "DIR.OMP.SIMD"()
// CHECK-SAME: "QUAL.OMP.SAFELEN"(i32 4)
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[IV]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"
// CHECK: region.exit(token [[T4]]) [ "DIR.OMP.END.SIMD"() ]
  #pragma omp simd safelen(4)
  for (iter = first1(); iter < last1(); ++iter) {
    arr1[iter] = 42+iter;
  }

// CHECK: [[T24:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: [ "DIR.OMP.PARALLEL.LOOP"(),
// CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[ARR2]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[IV11]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"
// CHECK: region.exit(token [[T24]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for collapse(2)
  for (i=first2(); i<last2(); ++i)
    for (j=first3(); j<last3(); ++j)
      arr2[i][j] = 42+i+j;

// CHECK: [[T42:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: [ "DIR.OMP.PARALLEL.LOOP"(),
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[IV72]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"
// CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[K]]
// CHECK: region.exit(token [[T42]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for
  for (int k=0; k<10; k++) {
    bar(k);
  }

// CHECK: [[T49:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: [ "DIR.OMP.LOOP"(),
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[IV88]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"
// CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[K97]]
// CHECK: region.exit(token [[T49]]) [ "DIR.OMP.END.LOOP"() ]
  #pragma omp for
  for (int k=0; k<10; k++) {
    bar(k);
  }

// CHECK: [[T61:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: [ "DIR.OMP.PARALLEL.LOOP"()
// CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[ARR1]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[IV105]]
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"

// CHECK: [[T62:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: [ "DIR.OMP.SIMD"()
// CHECK-SAME: "QUAL.OMP.SAFELEN"(i32 4)
// CHECK: region.exit(token [[T62]]) [ "DIR.OMP.END.SIMD"() ]
// CHECK: region.exit(token [[T61]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for simd safelen(4)
  for (iter = first1(); iter < last1(); ++iter) {
    int pr = 4;
    arr1[iter] = 42+iter+pr;
  }

  int zii = 0;
// CHECK: DIR.OMP.TASKLOOP{{.*}}REDUCTION.ADD
 #pragma omp taskloop reduction(+:zii)
 for (iter = first1(); iter < last1(); ++iter) {
   int pr = 4;
   arr1[iter] = 42+iter+pr;
   zii += 42+iter+pr;
 }

// CHECK: DIR.OMP.TASKLOOP{{.*}}REDUCTION.ADD
// CHECK: DIR.OMP.SIMD
 #pragma omp taskloop simd reduction(+:zii)
 for (iter = first1(); iter < last1(); ++iter) {
   int pr = 4;
   arr1[iter] = 42+iter+pr;
   zii += 42+iter+pr;
 }

}

// CHECK: define {{.*}}multiloop
void multiloop(int in, int *arr)
{
  // CHECK: [[OMP_UB1:%.omp.ub.*]] = alloca i32,
  // CHECK: [[OMP_UB2:%.omp.ub.*]] = alloca i32,
  // CHECK: [[OMP_UB3:%.omp.ub.*]] = alloca i32,
  // CHECK: [[OMP_UB4:%.omp.ub.*]] = alloca i32,
  // CHECK: [[OMP_UB5:%.omp.ub.*]] = alloca i32,
  // CHECK: [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_UB1]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_UB2]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_UB3]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_UB4]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_UB5]]
  // CHECK: [ "DIR.OMP.END.PARALLEL"() ]
  #pragma omp parallel
  {
    //1
    #pragma omp simd
    for(int i=0;i<16;++i)
      arr[i] = in;

    //2
    int i;
    #pragma omp simd
    for(i=0;i<16;++i)
      arr[i] = in;

    //3
    #pragma omp for
    for(i=0;i<16;++i)
      arr[i] = in;

    //4
    #pragma omp parallel for
    for(i=0;i<16;++i)
      arr[i] = in;

    //5
    #pragma omp parallel for simd
    for(i=0;i<16;++i)
      arr[i] = in;
  }
}

// CHECK: define{{.*}}doacross_test
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
  // CHECK: store i32 2, ptr [[VARJ]]

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

void i_inc_test_call(unsigned i);
//CHECK-LABEL: nuw_incr_test
void nuw_incr_test(unsigned n1, int ldf, int* buf)
{
  //CHECK: [[OMPIV:%.omp.iv[0-9]*]] = alloca i32,
  //CHECK: [[JJ:%j[0-9]*]] = alloca i32,

  //CHECK: "DIR.OMP.PARALLEL.LOOP"()
  #pragma omp parallel for
  for (unsigned r = 0; r < n1; ++r) {
     i_inc_test_call(r);
  }
  //CHECK: {{call|invoke}}{{.*}}i_inc_test_call
  // Expect 'add nuw' for this increment.
  //CHECK: [[LL:%[0-9]+]] = load i32, ptr [[OMPIV]]
  //CHECK: [[ADD:%add[0-9]*]] = add nuw i32 [[LL]], 1
  //CHECK: store i32 [[ADD]], ptr [[OMPIV]]
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"()

  // Just check that this increment is not affected.
  unsigned int j = 0;
  j = j + 1;
  //CHECK: store i32 0, ptr [[JJ]]
  //CHECK: [[L2:%[0-9]+]] = load i32, ptr [[JJ]]
  //CHECK: [[A2:%add[0-9]*]] = add i32 [[L2]], 1
  //CHECK: store i32 [[A2]], ptr [[JJ]]
}
// end INTEL_COLLAB
