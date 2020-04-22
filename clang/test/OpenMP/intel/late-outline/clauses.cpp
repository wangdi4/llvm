// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
// RUN: %clang_cc1 -disable-llvm-passes -emit-llvm -o - \
// RUN:  -fopenmp -fopenmp-late-outline -O2 \
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
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[IF1_ADDR]])
  // CHECK-SAME: "QUAL.OMP.IF"(i1 true)
  #pragma omp parallel private(if1) if(1)
  { foo(); }

  // CHECK: [[ILOAD1:%.+]] = load i32, i32* [[IF_VAL_ADDR]]
  // CHECK-NEXT: [[TOBOOL:%.+]] = icmp ne i32 [[ILOAD1]], 0
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[IF2_ADDR]])
  // CHECK-SAME: "QUAL.OMP.IF"(i1 [[TOBOOL]])
  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL"
  #pragma omp parallel private(if2) if(if_val)
  { foo(); }

  // CHECK: [[L1:%.+]] = load i32, i32* [[IF1_ADDR]]
  // CHECK-NEXT: [[TB2:%.+]] = icmp ne i32 [[L1]], 0
  // CHECK-NEXT: br i1 [[TB2]]
  // CHECK: [[L2:%.+]] = load i32, i32* [[IF2_ADDR]]
  // CHECK-NEXT: [[TB3:%.+]] = icmp ne i32 [[L2]], 0
  // CHECK: [[P5:%.+]] = phi i1
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.IF"(i1 [[P5]])
  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL"
  #pragma omp parallel if(if1 && if2)
  { foo(); }

  // proc_bind
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[PB1_ADDR]])
  // CHECK-SAME: "QUAL.OMP.PROC_BIND.MASTER"
  #pragma omp parallel private(pb1) proc_bind(master)
  { foo(); }

  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[PB2_ADDR]])
  // CHECK-SAME: "QUAL.OMP.PROC_BIND.CLOSE"
  #pragma omp parallel private(pb2) proc_bind(close)
  { foo(); }

  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[PB3_ADDR]])
  // CHECK-SAME: "QUAL.OMP.PROC_BIND.SPREAD"
  #pragma omp parallel private(pb3) proc_bind(spread)
  { foo(); }

  // num_threads
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[NT1_ADDR]])
  // CHECK-SAME: "QUAL.OMP.NUM_THREADS"(i32 8)
  #pragma omp parallel private(nt1) num_threads(8)
  { foo(); }

  // CHECK: [[ILOAD2:%.*]] = load i32, i32* [[NUM_THREADS_VAL_ADDR]]
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[NT2_ADDR]])
  // CHECK-SAME: "QUAL.OMP.NUM_THREADS"(i32 [[ILOAD2]])
  #pragma omp parallel private(nt2) num_threads(num_threads_val)
  { foo(); }

  // default
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[DF1_ADDR]])
  // CHECK-SAME: "QUAL.OMP.DEFAULT.NONE"
  #pragma omp parallel private(df1) default(none)
  { foo(); }

  // CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[DF2_ADDR]])
  // CHECK-SAME: "QUAL.OMP.DEFAULT.SHARED"
  #pragma omp parallel private(df2) default(shared)
  { foo(); }
}

//OPT-LABEL: @_Z4bar2
void bar2()
{
  //OPT: [[CDS:%cleanup.dest.slot.*]] = alloca i32,
  //OPT: region.entry() [ "DIR.OMP.PARALLEL"()
  //OPT-SAME: "QUAL.OMP.PRIVATE"(i32* [[CDS]])
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
  //OPT-SAME: "QUAL.OMP.PRIVATE"(i32* [[CDS]])
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
  //CHECK-SAME: "QUAL.OMP.SHARED"(i32* @_ZZ4bar3vE5st_b3)
  //CHECK-SAME: "QUAL.OMP.SHARED"(i32* @_ZL5st_b1_{{[0-9a-f]+}})
  //CHECK: "DIR.OMP.END.PARALLEL"
  #pragma omp parallel
  {
    static int st_b3;
    baz(st_b1+st_b3);
  }

  //CHECK: region.entry() [ "DIR.OMP.SIMD"()
  //CHECK-NOT: "QUAL.OMP.SHARED"
  //CHECK-NOT: "QUAL.OMP.PRIVATE"
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
  //CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[I1]])
  //CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[I1]])
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  //CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target map(alloc:Anext[0:nx])
  #pragma omp parallel for
  for(int i1=1;i1<nx-1;i1++) {
    Anext[i1] = c0;
  }
  //CHECK: region.entry() [ "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[I2]])
  //CHECK-NOT: "DIR.OMP.SIMD"(){{.*}}"QUAL.OMP.PRIVATE"(i32* [[I2]])
  //CHECK: "DIR.OMP.END.SIMD"
  //CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target map(alloc:Anext[0:nx])
  #pragma omp simd
  for(int i2=1;i2<nx-1;i2++) {
    Anext[i2] = c0;
  }
  //CHECK: region.entry() [ "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[I3]])
  //CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[I3]])
  //CHECK-NOT: "DIR.OMP.SIMD"(){{.*}}"QUAL.OMP.PRIVATE"(i32* [[I3]])
  //CHECK: "DIR.OMP.END.SIMD"
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  //CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target map(alloc:Anext[0:nx])
  #pragma omp parallel for simd
  for(int i3=1;i3<nx-1;i3++) {
    Anext[i3] = c0;
  }
  //CHECK: region.entry() [ "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[J1]])
  //CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[J1]])
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  //CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target map(alloc:Anext[0:nx])
  #pragma omp parallel for
  for(j1=1;j1<nx-1;j1++) {
    Anext[j1] = c0;
  }
  //CHECK: region.entry() [ "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[J2]])
  //CHECK: region.entry() [ "DIR.OMP.SIMD"()
  //CHECK: "DIR.OMP.END.SIMD"
  //CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target map(alloc:Anext[0:nx])
  #pragma omp simd
  for(j2=1;j2<nx-1;j2++) {
    Anext[j2] = c0;
  }
  //CHECK: region.entry() [ "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[J3]])
  //CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[J3]])
  //CHECK: region.entry() [ "DIR.OMP.SIMD"()
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
  //CHECK-DAG: "QUAL.OMP.MAP.TO"([9 x i32]* @bar6BBB, i32* getelementptr inbounds ([9 x i32], [9 x i32]* @bar6BBB, i64 0, i64 5), i64 8, i64 33)
  //CHECK-DAG: "QUAL.OMP.MAP.TO"([9 x i32]* @bar6AAA, i32* getelementptr inbounds ([9 x i32], [9 x i32]* @bar6AAA, i64 0, i64 2), i64 16, i64 33)
  //CHECK: "DIR.OMP.END.TARGET.UPDATE"()
  #pragma omp target update to(bar6BBB[5:2], bar6AAA[2:4])
  //CHECK: "DIR.OMP.TARGET.UPDATE"()
  //CHECK-DAG: QUAL.OMP.MAP.TO"([9 x i32]* @bar6BBB, i32* getelementptr inbounds ([9 x i32], [9 x i32]* @bar6BBB, i64 0, i64 2), i64 12, i64 33)
  //CHECK-DAG: "QUAL.OMP.MAP.TO"([9 x i32]* @bar6AAA, i32* getelementptr inbounds ([9 x i32], [9 x i32]* @bar6AAA, i64 0, i64 6), i64 4, i64 33)
  //CHECK: "DIR.OMP.END.TARGET.UPDATE"()
  #pragma omp target update to(bar6BBB[2:3], bar6AAA[6])
  //CHECK: "DIR.OMP.TARGET.UPDATE"()
  //CHECK-DAG: "QUAL.OMP.MAP.FROM"([9 x i32]* @bar6BBB, i32* getelementptr inbounds ([9 x i32], [9 x i32]* @bar6BBB, i64 0, i64 5), i64 8, i64 34)
  //CHECK-DAG: "QUAL.OMP.MAP.FROM"([9 x i32]* @bar6AAA, i32* getelementptr inbounds ([9 x i32], [9 x i32]* @bar6AAA, i64 0, i64 2), i64 12, i64 34)
  //CHECK: "DIR.OMP.END.TARGET.UPDATE"()
  #pragma omp target update from(bar6BBB[5:2], bar6AAA[2:3])
  //CHECK: "DIR.OMP.TARGET.UPDATE"()
  //CHECK-DAG: "QUAL.OMP.MAP.FROM"([9 x i32]* @bar6BBB, i32* getelementptr inbounds ([9 x i32], [9 x i32]* @bar6BBB, i64 0, i64 2), i64 12, i64 34)
  //CHECK-DAG: "QUAL.OMP.MAP.FROM"([9 x i32]* @bar6AAA, i32* getelementptr inbounds ([9 x i32], [9 x i32]* @bar6AAA, i64 0, i64 6), i64 4, i64 34)
  //CHECK: "DIR.OMP.END.TARGET.UPDATE"()
  #pragma omp target update from(bar6BBB[2:3], bar6AAA[6])
  //CHECK: "DIR.OMP.TARGET.UPDATE"()
  //CHECK-SAME: "QUAL.OMP.MAP.TO"(i32* @B_bar6, i32* @B_bar6, i64 4, i64 33)
  //CHECK-SAME: "QUAL.OMP.MAP.FROM"(i32* @A_bar6, i32* @A_bar6, i64 4, i64 34)
  //CHECK: "DIR.OMP.END.TARGET.UPDATE"()
  #pragma omp target update from(A_bar6) to(B_bar6)
}

// end INTEL_COLLAB
