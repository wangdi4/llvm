// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
// RUN: %clang_cc1 -opaque-pointers -disable-llvm-passes -emit-llvm -o - \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses -O2 \
// RUN:  -triple x86_64-unknown-linux-gnu %s \
// RUN: | FileCheck %s --check-prefix OPT

int foo();

// CHECK-LABEL: @_Z3barii
// CHECK: [[IF_VAL_ADDR:%.+]] = alloca i32,
// CHECK: [[NUM_THREADS_VAL_ADDR:%.+]] = alloca i32,
void bar(int if_val, int num_threads_val) {
  // CHECK: [[IF1_ADDR:%.+]] = alloca i32,
  int if1 = 1;
  // CHECK: [[IF2_ADDR:%.+]] = alloca i32,
  int if2 = 2;
  // CHECK: [[PB1_ADDR:%.+]] = alloca i32,
  int pb1 = 1;
  // CHECK: [[PB2_ADDR:%.+]] = alloca i32,
  int pb2 = 2;
  // CHECK: [[PB3_ADDR:%.+]] = alloca i32,
  int pb3 = 3;
  // CHECK: [[NT1_ADDR:%.+]] = alloca i32,
  int nt1 = 1;
  // CHECK: [[NT2_ADDR:%.+]] = alloca i32,
  int nt2 = 2;
  // CHECK: [[DF1_ADDR:%.+]] = alloca i32,
  int df1 = 1;
  // CHECK: [[DF2_ADDR:%.+]] = alloca i32,
  int df2 = 2;

  // if
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[IF1_ADDR]]
  // CHECK-SAME: "QUAL.OMP.IF"(i1 true)
  #pragma omp parallel private(if1) if(1)
  { foo(); }

  // CHECK: [[ILOAD1:%.+]] = load i32, ptr [[IF_VAL_ADDR]]
  // CHECK-NEXT: [[TOBOOL:%.+]] = icmp ne i32 [[ILOAD1]], 0
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[IF2_ADDR]]
  // CHECK-SAME: "QUAL.OMP.IF"(i1 [[TOBOOL]])
  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL"
  #pragma omp parallel private(if2) if(if_val)
  { foo(); }

  // CHECK: [[L1:%.+]] = load i32, ptr [[IF1_ADDR]]
  // CHECK-NEXT: [[TB2:%.+]] = icmp ne i32 [[L1]], 0
  // CHECK-NEXT: br i1 [[TB2]]
  // CHECK: [[L2:%.+]] = load i32, ptr [[IF2_ADDR]]
  // CHECK-NEXT: [[TB3:%.+]] = icmp ne i32 [[L2]], 0
  // CHECK: [[P5:%.+]] = phi i1
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.IF"(i1 [[P5]])
  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL"
  #pragma omp parallel if(if1 && if2)
  { foo(); }

  // proc_bind
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[PB1_ADDR]]
  // CHECK-SAME: "QUAL.OMP.PROC_BIND.MASTER"
  #pragma omp parallel private(pb1) proc_bind(master)
  { foo(); }

  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[PB2_ADDR]]
  // CHECK-SAME: "QUAL.OMP.PROC_BIND.CLOSE"
  #pragma omp parallel private(pb2) proc_bind(close)
  { foo(); }

  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[PB3_ADDR]]
  // CHECK-SAME: "QUAL.OMP.PROC_BIND.SPREAD"
  #pragma omp parallel private(pb3) proc_bind(spread)
  { foo(); }

  // num_threads
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[NT1_ADDR]]
  // CHECK-SAME: "QUAL.OMP.NUM_THREADS"(i32 8)
  #pragma omp parallel private(nt1) num_threads(8)
  { foo(); }

  // CHECK: [[ILOAD2:%.*]] = load i32, ptr [[NUM_THREADS_VAL_ADDR]]
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[NT2_ADDR]]
  // CHECK-SAME: "QUAL.OMP.NUM_THREADS"(i32 [[ILOAD2]])
  #pragma omp parallel private(nt2) num_threads(num_threads_val)
  { foo(); }

  // default
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[DF1_ADDR]]
  // CHECK-SAME: "QUAL.OMP.DEFAULT.NONE"
  #pragma omp parallel private(df1) default(none)
  { foo(); }

  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[DF2_ADDR]]
  // CHECK-SAME: "QUAL.OMP.DEFAULT.SHARED"
  #pragma omp parallel private(df2) default(shared)
  { foo(); }
}

//OPT-LABEL: @_Z4bar2
void bar2()
{
  //OPT: [[CDS:%cleanup.dest.slot.*]] = alloca i32,
  //OPT: region.entry() [ "DIR.OMP.PARALLEL"()
  //OPT-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[CDS]]
  //OPT: "DIR.OMP.END.PARALLEL"
  #pragma omp parallel
  {
    for ( int y = 0 ; y < 100 ; y++ )
    {
      for (int x = 0 ; x < 100 ; x++ )
      {
      }
    }
  }

  //OPT: region.entry() [ "DIR.OMP.PARALLEL"()
  //OPT-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[CDS]]
  //OPT: "DIR.OMP.END.PARALLEL"
  #pragma omp parallel
  {
    for ( int i = 0 ; i < 8 ; i++ )
    {
    }
  }
}

static int st_b1;
void baz(int);

//CHECK-LABEL: bar3
void bar3()
{
  //CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr @_ZL5st_b1
  //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr @_ZZ4bar3vE5st_b3
  //CHECK: "DIR.OMP.END.PARALLEL"
  #pragma omp parallel
  {
    static int st_b3;
    baz(st_b1+st_b3);
  }

  //CHECK: region.entry() [ "DIR.OMP.SIMD"()
  //CHECK-NOT: "QUAL.OMP.SHARED:TYPED"
  //CHECK-NOT: "QUAL.OMP.PRIVATE:TYPED"
  //CHECK: "DIR.OMP.END.SIMD"
  #pragma omp simd
  for (int i=0;i<16;++i) {
    static int st_b4;
    baz(st_b1+st_b4);
  }
}

//CHECK-LABEL: bar4
void bar4(float c0, float *Anext, const int nx)
{
  int j1,j2,j3;
  //CHECK: [[J1:%j1.*]] = alloca i32,
  //CHECK: [[J2:%j2.*]] = alloca i32,
  //CHECK: [[J3:%j3.*]] = alloca i32,
  //CHECK: [[I1:%i1.*]] = alloca i32,
  //CHECK: [[I2:%i2.*]] = alloca i32,
  //CHECK: [[I3:%i3.*]] = alloca i32,

  //CHECK: region.entry() [ "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[I1]]
  //CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[I1]]
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  //CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target map(alloc:Anext[0:nx])
  #pragma omp parallel for
  for(int i1=1;i1<nx-1;i1++) {
    Anext[i1] = c0;
  }
  //CHECK: region.entry() [ "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[I2]]
  //CHECK-NOT: "DIR.OMP.SIMD"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[I2]]
  //CHECK: "DIR.OMP.END.SIMD"
  //CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target map(alloc:Anext[0:nx])
  #pragma omp simd
  for(int i2=1;i2<nx-1;i2++) {
    Anext[i2] = c0;
  }
  //CHECK: region.entry() [ "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[I3]]
  //CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[I3]]
  //CHECK-NOT: "DIR.OMP.SIMD"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[I3]]
  //CHECK: "DIR.OMP.END.SIMD"
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  //CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target map(alloc:Anext[0:nx])
  #pragma omp parallel for simd
  for(int i3=1;i3<nx-1;i3++) {
    Anext[i3] = c0;
  }
  //CHECK: region.entry() [ "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[J1]]
  //CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[J1]]
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  //CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target map(alloc:Anext[0:nx])
  #pragma omp parallel for
  for(j1=1;j1<nx-1;j1++) {
    Anext[j1] = c0;
  }
  //CHECK: region.entry() [ "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[J2]]
  //CHECK: region.entry() [ "DIR.OMP.SIMD"()
  //CHECK: "DIR.OMP.END.SIMD"
  //CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target map(alloc:Anext[0:nx])
  #pragma omp simd
  for(j2=1;j2<nx-1;j2++) {
    Anext[j2] = c0;
  }
  //CHECK: region.entry() [ "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[J3]]
  //CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.LASTPRIVATE:TYPED"(ptr [[J3]]
  //CHECK: region.entry() [ "DIR.OMP.SIMD"()
  //CHECK-SAME: "QUAL.OMP.LINEAR:IV.TYPED"
  //CHECK: "DIR.OMP.END.SIMD"
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  //CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target map(alloc:Anext[0:nx])
  #pragma omp parallel for simd
  for(j3=1;j3<nx-1;j3++) {
    Anext[j3] = c0;
  }
}

//CHECK-LABEL: bar5
void bar5()
{
  int i,sx;
  // CHECK-NOT: call token{{.*}}"DIR.OMP.TARGET"{{.*}}REDUCTION
  // CHECK: [[T1:%[0-9]+]] = call token{{.*}} "DIR.OMP.TEAMS"
  // CHECK: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  #pragma omp target teams distribute parallel for reduction(+:sx)
  for(i=0;i<10;++i) {}
  // CHECK: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"
  // CHECK: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  // CHECK: "DIR.OMP.END.TARGET"
}

//CHECK-LABEL: bar6
int bar6AAA[9];
int bar6BBB[9];
int A_bar6, B_bar6;
void bar6()
{
  //CHECK: "DIR.OMP.TARGET.UPDATE"()
  //CHECK-DAG: "QUAL.OMP.MAP.TO"(ptr @bar6BBB, ptr getelementptr inbounds ([9 x i32], ptr @bar6BBB, i64 0, i64 5), i64 8, i64 1
  //CHECK-DAG: "QUAL.OMP.MAP.TO"(ptr @bar6AAA, ptr getelementptr inbounds ([9 x i32], ptr @bar6AAA, i64 0, i64 2), i64 16, i64 1
  //CHECK: "DIR.OMP.END.TARGET.UPDATE"()
  #pragma omp target update to(bar6BBB[5:2], bar6AAA[2:4])
  //CHECK: "DIR.OMP.TARGET.UPDATE"()
  //CHECK-DAG: "QUAL.OMP.MAP.TO"(ptr @bar6BBB, ptr getelementptr inbounds ([9 x i32], ptr @bar6BBB, i64 0, i64 2), i64 12, i64 1
  //CHECK-DAG: "QUAL.OMP.MAP.TO"(ptr @bar6AAA, ptr getelementptr inbounds ([9 x i32], ptr @bar6AAA, i64 0, i64 6), i64 4, i64 1
  //CHECK: "DIR.OMP.END.TARGET.UPDATE"()
  #pragma omp target update to(bar6BBB[2:3], bar6AAA[6])
  //CHECK: "DIR.OMP.TARGET.UPDATE"()
  //CHECK-DAG: "QUAL.OMP.MAP.FROM"(ptr @bar6BBB, ptr getelementptr inbounds ([9 x i32], ptr @bar6BBB, i64 0, i64 5), i64 8, i64 2
  //CHECK-DAG: "QUAL.OMP.MAP.FROM"(ptr @bar6AAA, ptr getelementptr inbounds ([9 x i32], ptr @bar6AAA, i64 0, i64 2), i64 12, i64 2
  //CHECK: "DIR.OMP.END.TARGET.UPDATE"()
  #pragma omp target update from(bar6BBB[5:2], bar6AAA[2:3])
  //CHECK: "DIR.OMP.TARGET.UPDATE"()
  //CHECK-DAG: "QUAL.OMP.MAP.FROM"(ptr @bar6BBB, ptr getelementptr inbounds ([9 x i32], ptr @bar6BBB, i64 0, i64 2), i64 12, i64 2
  //CHECK-DAG: "QUAL.OMP.MAP.FROM"(ptr @bar6AAA, ptr getelementptr inbounds ([9 x i32], ptr @bar6AAA, i64 0, i64 6), i64 4, i64 2
  //CHECK: "DIR.OMP.END.TARGET.UPDATE"()
  #pragma omp target update from(bar6BBB[2:3], bar6AAA[6])
  //CHECK: "DIR.OMP.TARGET.UPDATE"()
  //CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr @B_bar6, ptr @B_bar6, i64 4, i64 1
  //CHECK-SAME: "QUAL.OMP.MAP.FROM"(ptr @A_bar6, ptr @A_bar6, i64 4, i64 2
  //CHECK: "DIR.OMP.END.TARGET.UPDATE"()
  #pragma omp target update from(A_bar6) to(B_bar6)
}

void call_bar7(float *);
//CHECK-LABEL: bar7
void bar7(float *A, int N, int S)
{
  //CHECK: [[I:%i.*]] = alloca i32,
  int i;
  //CHECK: [[ULL:%ull.*]] = alloca i64,
  unsigned long long ull;
  //CHECK: [[FP:%fp.*]] = alloca ptr,
  float *fp;

  // step 1
  //CHECK: "DIR.OMP.SIMD"
  //CHECK-SAME: "QUAL.OMP.LINEAR:IV.TYPED"(ptr [[I]], i32 0, i32 1, i32 1)
  //CHECK: call void {{.*}}call_bar7
  //CHECK: load i32, ptr %.omp.iv
  //CHECK-NEXT: add nsw i32 {{.*}}1{{$}}
  //CHECK-NEXT: store i32 {{.*}}%.omp.iv
  //CHECK-NEXT: [[L1:%[0-9]+]] = load i32, ptr [[I]]
  //CHECK-NEXT: [[ADD1:%add[0-9]*]] = add nsw i32 [[L1]], 1
  //CHECK-NEXT: store i32 [[ADD1]], ptr [[I]]
  #pragma omp simd
  for (i=0;i<N;i++) { call_bar7(&A[i]); }
  //CHECK: "DIR.OMP.END.SIMD"

  // step -1
  //CHECK: "DIR.OMP.SIMD"
  //CHECK-SAME: "QUAL.OMP.LINEAR:IV.TYPED"(ptr [[I]], i32 0, i32 1, i32 -1)
  //CHECK: call void {{.*}}call_bar7
  //CHECK: load i32, ptr %.omp.iv
  //CHECK-NEXT: add nuw i32 {{.*}}1{{$}}
  //CHECK-NEXT: store i32 {{.*}}%.omp.iv
  //CHECK-NEXT: [[L1:%[0-9]+]] = load i32, ptr [[I]]
  //CHECK-NEXT: [[SUB1:%sub[0-9]*]] = sub nsw i32 [[L1]], 1
  //CHECK-NEXT: store i32 [[SUB1]], ptr [[I]]
  #pragma omp simd
  for (i=N;i>=0;i--) { call_bar7(&A[i]); }
  //CHECK: "DIR.OMP.END.SIMD"

  // step 4
  //CHECK: "DIR.OMP.SIMD"
  //CHECK-SAME: "QUAL.OMP.LINEAR:IV.TYPED"(ptr [[I]], i32 0, i32 1, i32 4)
  //CHECK: call void {{.*}}call_bar7
  //CHECK: load i32, ptr %.omp.iv
  //CHECK-NEXT: add {{.*}} i32 {{.*}}1{{$}}
  //CHECK-NEXT: store i32 {{.*}}%.omp.iv
  //CHECK-NEXT: [[L1:%[0-9]+]] = load i32, ptr [[I]]
  //CHECK-NEXT: [[ADD1:%add[0-9]*]] = add nsw i32 [[L1]], 4
  //CHECK-NEXT: store i32 [[ADD1]], ptr [[I]]
  #pragma omp simd
  for (i=0;i<N;i+=4) { call_bar7(&A[i]); }
  //CHECK: "DIR.OMP.END.SIMD"

  // step -8
  //CHECK: "DIR.OMP.SIMD"
  //CHECK-SAME: "QUAL.OMP.LINEAR:IV.TYPED"(ptr [[I]], i32 0, i32 1, i32 -8)
  //CHECK: call void {{.*}}call_bar7
  //CHECK: load i32, ptr %.omp.iv
  //CHECK-NEXT: add nuw i32 {{.*}}1{{$}}
  //CHECK-NEXT: store i32 {{.*}}%.omp.iv
  //CHECK-NEXT: [[L1:%[0-9]+]] = load i32, ptr [[I]]
  //CHECK-NEXT: [[SUB1:%sub[0-9]*]] = sub nsw i32 [[L1]], 8
  //CHECK-NEXT: store i32 [[SUB1]], ptr [[I]]
  #pragma omp simd
  for (i=N;i>=0;i-=8) { call_bar7(&A[i]); }
  //CHECK: "DIR.OMP.END.SIMD"

  // another form: step -8
  //CHECK: "DIR.OMP.SIMD"
  //CHECK-SAME: "QUAL.OMP.LINEAR:IV.TYPED"(ptr [[I]], i32 0, i32 1, i32 -8)
  //CHECK: call void {{.*}}call_bar7
  //CHECK: load i32, ptr %.omp.iv
  //CHECK-NEXT: add nuw i32 {{.*}}1{{$}}
  //CHECK-NEXT: store i32 {{.*}}%.omp.iv
  //CHECK-NEXT: [[L1:%[0-9]+]] = load i32, ptr [[I]]
  //CHECK-NEXT: [[SUB1:%sub[0-9]*]] = sub nsw i32 [[L1]], 8
  //CHECK-NEXT: store i32 [[SUB1]], ptr [[I]]
  #pragma omp simd
  for (i=N;i>=0;i=i-8) { call_bar7(&A[i]); }
  //CHECK: "DIR.OMP.END.SIMD"

  //CHECK: [[LC:%[0-9]+]] = load i32, ptr %S.addr,
  //CHECK-NEXT: store i32 [[LC]], ptr [[CAPS:%.capture_expr.[0-9]*]]

  // non-constant step
  //CHECK: icmp slt i32 0,
  //CHECK: [[LC:%[0-9]+]] = load i32, ptr [[CAPS]],
  //CHECK: "DIR.OMP.SIMD"
  //CHECK-SAME: "QUAL.OMP.LINEAR:IV.TYPED"(ptr [[I]], i32 0, i32 1, i32 [[LC]])
  //CHECK: call void {{.*}}call_bar7
  //CHECK: load i32, ptr %.omp.iv
  //CHECK-NEXT: add nsw i32 {{.*}}1{{$}}
  //CHECK-NEXT: store i32 {{.*}}%.omp.iv
  //CHECK-NEXT: [[L1:%[0-9]+]] = load i32, ptr [[I]]
  //CHECK-NEXT: [[L2:%[0-9]+]] = load i32, ptr [[CAPS]]
  //CHECK-NEXT: [[ADD1:%add[0-9]*]] = add nsw i32 [[L1]], [[L2]]
  //CHECK-NEXT: store i32 [[ADD1]], ptr [[I]]
  #pragma omp simd
  for (i=0;i<N;i+=S) { call_bar7(&A[i]); }
  //CHECK: "DIR.OMP.END.SIMD"

  // non-int
  //CHECK: "DIR.OMP.SIMD"
  //CHECK-SAME: "QUAL.OMP.LINEAR:IV.TYPED"(ptr [[ULL]], i64 0, i32 1, i32 -8)
  //CHECK: call void {{.*}}call_bar7
  //CHECK: load i64, ptr %.omp.iv
  //CHECK-NEXT: add nuw i64 {{.*}}1{{$}}
  //CHECK-NEXT: store i64 {{.*}}%.omp.iv
  //CHECK-NEXT: [[L1:%[0-9]+]] = load i64, ptr [[ULL]]
  //CHECK-NEXT: [[SUB1:%sub[0-9]*]] = sub i64 [[L1]], 8
  //CHECK-NEXT: store i64 [[SUB1]], ptr [[ULL]]
  #pragma omp simd
  for (ull=N;ull>=0;ull-=8) { call_bar7(&A[ull]); }
  //CHECK: "DIR.OMP.END.SIMD"

  //CHECK: "DIR.OMP.SIMD"
  //CHECK-SAME: "QUAL.OMP.LINEAR:IV.TYPED"(ptr [[FP]], ptr null, i32 1, i32 4)
  //CHECK: call void {{.*}}call_bar7
  //CHECK: load i64, ptr %.omp.iv
  //CHECK-NEXT: add nsw i64 {{.*}}1{{$}}
  //CHECK-NEXT: store i64 {{.*}}%.omp.iv
  //CHECK-NEXT: [[L1:%[0-9]+]] = load ptr, ptr [[FP]]
  //CHECK-NEXT: [[ADD1:%add.ptr[0-9]*]] = getelementptr inbounds float, ptr [[L1]], i64 4
  //CHECK-NEXT: store ptr [[ADD1]], ptr [[FP]]
  #pragma omp simd
  for (fp=&A[2];fp<&A[14];fp+=4) { call_bar7(fp); }
  //CHECK: "DIR.OMP.END.SIMD"

  //CHECK: "DIR.OMP.DISTRIBUTE.PARLOOP"
  //CHECK-SAME: "QUAL.OMP.LASTPRIVATE:TYPED"(ptr [[I]]
  //CHECK: "DIR.OMP.SIMD"
  //CHECK-SAME: "QUAL.OMP.LINEAR:IV.TYPED"(ptr [[I]], i32 0, i32 1, i32 4)
  #pragma omp distribute parallel for simd
  for (i=0;i<N;i+=4) { call_bar7(&A[i]); }
  //CHECK: "DIR.OMP.END.SIMD"
  //CHECK: "DIR.OMP.END.DISTRIBUTE.PARLOOP"
}

// Test there are no PRIVATE clauses on SIMD for the loop counter.
#define LOOP for (int x = 0; x < 20; ++x) arr[x] = x;

//CHECK-LABEL: bar8
void bar8(int *arr)
{
  //CHECK: "DIR.OMP.SIMD"
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: QUAL.OMP.LINEAR:IV
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: "DIR.OMP.END.SIMD"
  #pragma omp parallel for simd
  LOOP

  //CHECK: "DIR.OMP.SIMD"
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: QUAL.OMP.LINEAR:IV
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: "DIR.OMP.END.SIMD"
  #pragma omp for simd
  LOOP

  //CHECK: "DIR.OMP.SIMD"
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: QUAL.OMP.LINEAR:IV
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: "DIR.OMP.END.SIMD"
  #pragma omp taskloop simd
  LOOP

  //CHECK: "DIR.OMP.SIMD"
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: QUAL.OMP.LINEAR:IV
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: "DIR.OMP.END.SIMD"
  #pragma omp distribute parallel for simd
  LOOP

  //CHECK: "DIR.OMP.SIMD"
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: QUAL.OMP.LINEAR:IV
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: "DIR.OMP.END.SIMD"
  #pragma omp distribute simd
  LOOP

  //CHECK: "DIR.OMP.SIMD"
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: QUAL.OMP.LINEAR:IV
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: "DIR.OMP.END.SIMD"
  #pragma omp target parallel for simd
  LOOP

  //CHECK: "DIR.OMP.SIMD"
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: QUAL.OMP.LINEAR:IV
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: "DIR.OMP.END.SIMD"
  #pragma omp target simd
  LOOP

  //CHECK: "DIR.OMP.SIMD"
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: QUAL.OMP.LINEAR:IV
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: "DIR.OMP.END.SIMD"
  #pragma omp target
  #pragma omp teams distribute simd
  LOOP

  //CHECK: "DIR.OMP.SIMD"
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: QUAL.OMP.LINEAR:IV
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: "DIR.OMP.END.SIMD"
  #pragma omp target
  #pragma omp teams distribute parallel for simd
  LOOP

  //CHECK: "DIR.OMP.SIMD"
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: QUAL.OMP.LINEAR:IV
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: "DIR.OMP.END.SIMD"
  #pragma omp target teams distribute parallel for simd
  LOOP

  //CHECK: "DIR.OMP.SIMD"
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: QUAL.OMP.LINEAR:IV
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: "DIR.OMP.END.SIMD"
  #pragma omp target teams distribute simd
  LOOP

  //CHECK: "DIR.OMP.SIMD"
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: QUAL.OMP.LINEAR:IV
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: "DIR.OMP.END.SIMD"
  #pragma omp master taskloop simd
  LOOP

  //CHECK: "DIR.OMP.SIMD"
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: QUAL.OMP.LINEAR:IV
  //CHECK-NOT: QUAL.OMP.PRIVATE
  //CHECK: "DIR.OMP.END.SIMD"
  #pragma omp parallel master taskloop simd
  LOOP
}

// end INTEL_COLLAB
