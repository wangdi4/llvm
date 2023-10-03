// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp \
// RUN:  -fopenmp-targets=spir64 -fopenmp-version=51\
// RUN:  -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

void boo(long long n) {
int local = 2;
  // CHECK: [[NADDR:%n.addr]] = alloca i64, align 8
  // CHECK: [[LOCAL:%local]] = alloca i32, align 4
  // CHECK: [[EXPR0:%.capture_expr.[0-9]*]] = alloca i32, align 4
  // CHECK: [[EXPR1:%.capture_expr.[0-9]*]] = alloca i64, align 8
  // CHECK: [[EXPR2:%.capture_expr.[0-9]*]] = alloca i32, align 4
  // CHECK: [[EXPR3:%.capture_expr.[0-9]*]] = alloca i64, align 8
  // CHECK: [[EXPR4:%.capture_expr.[0-9]*]] = alloca i32, align 4
  // CHECK: [[EXPR5:%.capture_expr.[0-9]*]] = alloca i32, align 4
  // CHECK: [[EXPR6:%.capture_expr.[0-9]*]] = alloca i32, align 4
  // CHECK: [[EXPR7:%.capture_expr.[0-9]*]] = alloca i32, align 4
  // CHECK: [[EXPR8:%.capture_expr.[0-9]*]] = alloca i32, align 4
  // CHECK: [[TEMP0:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32 2)
#pragma omp target thread_limit(2)
  {}
  // CHECK: region.exit(token [[TEMP0]]) [ "DIR.OMP.END.TARGET"() ]

  // CHECK: [[TEMP1:%[0-9]+]] = load i32, ptr [[LOCAL]], align 4
  // CHECK: store i32 [[TEMP1]], ptr [[EXPR0]], align 4
  // CHECK: [[TEMP3:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[EXPR0]], i32 0)
#pragma omp target thread_limit(local)
  {}
  // CHECK: region.exit(token [[TEMP3]]) [ "DIR.OMP.END.TARGET"() ]
  
  // CHECK: [[TEMP4:%[0-9]+]] = load i64, ptr [[NADDR]], align 8
  // CHECK: store i64 [[TEMP4]], ptr [[EXPR1]], align 8
  // CHECK: [[TEMP6:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[EXPR1]], i64 0)
#pragma omp target thread_limit(n)
  {}
  // CHECK: region.exit(token [[TEMP6]]) [ "DIR.OMP.END.TARGET"() ]

  // CHECK: [[TEMP7:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32 2)
  // CHECK: [[TEMP8:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TEAMS
  // CHECK-NOT: "QUAL.OMP.THREAD_LIMIT"
#pragma omp target teams thread_limit(2)
  {}
  // CHECK: region.exit(token [[TEMP8]]) [ "DIR.OMP.END.TEAMS"() ]
  // CHECK: region.exit(token [[TEMP7]]) [ "DIR.OMP.END.TARGET"() ]

  // CHECK: [[TEMP9:%[0-9]+]] = load i32, ptr [[LOCAL]], align 4
  // CHECK: store i32 [[TEMP9]], ptr [[EXPR2]], align 4
  // CHECK: [[TEMP10:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[EXPR2]], i32 0)
  // CHECK: [[TEMP11:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TEAMS
  // CHECK-NOT: "QUAL.OMP.THREAD_LIMIT"
#pragma omp target teams thread_limit(local)
  {}
  // CHECK: region.exit(token [[TEMP11]]) [ "DIR.OMP.END.TEAMS"() ]
  // CHECK: region.exit(token [[TEMP10]]) [ "DIR.OMP.END.TARGET"() ]

  // CHECK: [[TEMP12:%[0-9]+]] = load i64, ptr [[NADDR]], align 8
  // CHECK: store i64 [[TEMP12]], ptr [[EXPR3]], align 8
  // CHECK: [[TEMP13:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[EXPR3]], i64 0)
  // CHECK: [[TEMP14:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TEAMS
  // CHECK-NOT: "QUAL.OMP.THREAD_LIMIT"
#pragma omp target teams thread_limit(n)
  {}
  // CHECK: region.exit(token [[TEMP14]]) [ "DIR.OMP.END.TEAMS"() ]
  // CHECK: region.exit(token [[TEMP13]]) [ "DIR.OMP.END.TARGET"() ]

  // CHECK: [[TEMP15:%[0-9]+]] = load i32, ptr [[LOCAL]], align 4
  // CHECK: store i32 [[TEMP15]], ptr [[EXPR4]], align 4
  // CHECK: [[TEMP16:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[EXPR4]], i32 0)
  // CHECK: [[TEMP17:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TEAMS
  // CHECK-NOT: "QUAL.OMP.THREAD_LIMIT"
  // CHECK: [[TEMP18:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE
#pragma omp target teams distribute thread_limit(local)
  for(int i = 0; i < 1024; i++) {
  }
  // CHECK: region.exit(token [[TEMP18]]) [ "DIR.OMP.END.DISTRIBUTE"() ]
  // CHECK: region.exit(token [[TEMP17]]) [ "DIR.OMP.END.TEAMS"() ]
  // CHECK: region.exit(token [[TEMP16]]) [ "DIR.OMP.END.TARGET"() ]

  // CHECK: [[TEMP24:%[0-9]+]] = load i32, ptr [[LOCAL]], align 4
  // CHECK: store i32 [[TEMP24]], ptr [[EXPR5]], align 4
  // CHECK: [[TEMP25:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[EXPR5]], i32 0)
  // CHECK: [[TEMP26:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TEAMS
  // CHECK-NOT: "QUAL.OMP.THREAD_LIMIT"
  // CHECK: [[TEMP27:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE
  // CHECK: [[TEMP28:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.SIMD
#pragma omp target teams distribute simd thread_limit(local)
  for(int i = 0; i < 1024; i++) {
  }
  // CHECK: region.exit(token [[TEMP28]]) [ "DIR.OMP.END.SIMD"() ]
  // CHECK: region.exit(token [[TEMP27]]) [ "DIR.OMP.END.DISTRIBUTE"() ]
  // CHECK: region.exit(token [[TEMP26]]) [ "DIR.OMP.END.TEAMS"() ]
  // CHECK: region.exit(token [[TEMP25]]) [ "DIR.OMP.END.TARGET"() ]

  // CHECK: [[TEMP34:%[0-9]+]] = load i32, ptr [[LOCAL]], align 4
  // CHECK: store i32 [[TEMP34]], ptr [[EXPR6]], align 4
  // CHECK: [[TEMP35:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[EXPR6]], i32 0)
  // CHECK: [[TEMP36:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TEAMS
  // CHECK-NOT: "QUAL.OMP.THREAD_LIMIT"
  // CHECK: [[TEMP37:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE.PARLOOP"()
#pragma omp target teams distribute parallel for thread_limit(local)
  for(int i = 0; i < 1024; i++) {
  }
  // CHECK: region.exit(token [[TEMP37]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  // CHECK: region.exit(token [[TEMP36]]) [ "DIR.OMP.END.TEAMS"() ]
  // CHECK: region.exit(token [[TEMP35]]) [ "DIR.OMP.END.TARGET"() ]

  // CHECK: [[TEMP43:%[0-9]+]] = load i32, ptr [[LOCAL]], align 4
  // CHECK: store i32 [[TEMP43]], ptr [[EXPR7]], align 4
  // CHECK: [[TEMP44:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[EXPR7]], i32 0)
  // CHECK: [[TEMP45:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TEAMS
  // CHECK-NOT: "QUAL.OMP.THREAD_LIMIT"
  // CHECK: [[TEMP46:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE.PARLOOP
  // CHECK: [[TEMP47:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.SIMD
#pragma omp target teams distribute parallel for simd thread_limit(local)
  for(int i = 0; i < 1024; i++) {
  }
  // CHECK: region.exit(token [[TEMP47]]) [ "DIR.OMP.END.SIMD"() ]
  // CHECK: region.exit(token [[TEMP46]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  // CHECK: region.exit(token [[TEMP45]]) [ "DIR.OMP.END.TEAMS"() ]
  // CHECK: region.exit(token [[TEMP44]]) [ "DIR.OMP.END.TARGET"() ]

  // CHECK: [[TEMP53:%[0-9]+]] = load i32, ptr [[LOCAL]], align 4
  // CHECK: store i32 [[TEMP53]], ptr [[EXPR8]], align 4
  // CHECK: [[TEMP54:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[EXPR8]], i32 0)
  // CHECK: [[TEMP55:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TEAMS
  // CHECK-NOT: "QUAL.OMP.THREAD_LIMIT"
  // CHECK: [[TEMP56:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.GENERICLOOP
#pragma omp target teams loop thread_limit(local)
  for(int i = 0; i < 1024; i++) {
  }
  // CHECK: region.exit(token [[TEMP56]]) [ "DIR.OMP.END.GENERICLOOP"() ]
  // CHECK: region.exit(token [[TEMP55]]) [ "DIR.OMP.END.TEAMS"() ]
  // CHECK: region.exit(token [[TEMP54]]) [ "DIR.OMP.END.TARGET"() ]
}

struct ThreadData { int n;};

void ThreadsTeams(ThreadData* threads)
{
  // CHECK: [[EXPR9:%.capture_expr.9]] = alloca i32, align 4
  // CHECK: [[N:%n]] = getelementptr inbounds %struct.ThreadData,
  // CHECK: [[TMP1:%[0-9]+]] = load i32, ptr [[N]], align 4
  // CHECK: [[ADD:%add]] = add nsw i32 [[TMP1]], 2
  // CHECK: store i32 [[ADD]], ptr [[EXPR9]], align 4
  // CHECK: [[TMP2:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[EXPR9]], i32 0) ]
  // CHECK: [[TMP3:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TEAMS
  // CHECK-NOT: "QUAL.OMP.THREAD_LIMIT"
  #pragma omp target teams thread_limit(threads->n + 2)
   {}
  // CHECK: region.exit(token [[TMP3]]) [ "DIR.OMP.END.TEAMS"() ]
  // CHECK: region.exit(token [[TMP2]]) [ "DIR.OMP.END.TARGET"() ]
}
// end INTEL_COLLAB
