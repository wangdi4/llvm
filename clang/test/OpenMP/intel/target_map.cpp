// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -o - %s -fexceptions -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s

struct S1 {
  int y;
  double d[50];
  struct S1 *next;
};

void foo(S1 *ps1)
{
  // CHECK: [[PS1_ADDR:%.+]] = alloca %struct.S1*,
  // CHECK: [[A:%.+]] = alloca i32,
  // CHECK: [[B:%.+]] = alloca double,
  // CHECK: [[ARRS:%.+]] = alloca [99 x i32],
  // CHECK: [[ARRD:%.+]] = alloca [49 x [29 x i32]],
  int a; double b;
  int arrS[99];
  int arrD[49][29];

  // CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: MAP.TOFROM{{.*}}(i32* [[A]]),
  // CHECK-SAME: MAP.TOFROM{{.*}}(double* [[B]])
  // CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(a,b)
  {
    a = 1;
    b = 2;
  }

  // CHECK: [[L1:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[L2:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[P1:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L2]], i32 0, i32 0
  // CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: MAP.TOFROM:AGGRHEAD{{.*}}(%struct.S1* [[L1]], i32* [[P1]], i64 4)
  // CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(ps1->y)
  {
    ps1->y = 3;
  }

  // CHECK: [[L5:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[L6:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[N1:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L6]], i32 0, i32 2
  // CHECK: [[L7:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[N2:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L7]], i32 0, i32 2
  // CHECK: [[L8:%[0-9]+]] = load %struct.S1*, %struct.S1** [[N2]],
  // CHECK: [[Y3:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L8]], i32 0, i32 0
  // CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: MAP.TOFROM:AGGRHEAD"{{.*}}(%struct.S1** [[N1]], i32* [[Y3]], i64 4)
  // CHECK: region.exit(token [[TV3]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(ps1->next->y)
  {
    ps1->next->y = 4;
  }

  // CHECK: [[L12:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[L13:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[N6:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L13]], i32 0, i32 2
  // CHECK: [[L14:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[N7:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L14]], i32 0, i32 2
  // CHECK: [[L15:%[0-9]+]] = load %struct.S1*, %struct.S1** [[N7]],
  // CHECK: [[D0:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L15]], i32 0, i32 1
  // CHECK: [[AI:%.+]] = getelementptr inbounds [50 x double], [50 x double]* [[D0]], i64 0, i64 17
  // CHECK: [[TV4:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // DCHECK-SAME: MAP.TOFROM:AGGRHEAD"{{.*}}(%struct.S1** [[N6]], double* [[AI]], i64 200)
  // CHECK: region.exit(token [[TV4]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(ps1->next->d[17:25])
  {
    ps1->next->d[17] = 5;
    ps1->next->d[40] = 6;
  }

  // CHECK: [[AI14:%.+]] = getelementptr inbounds [99 x i32], [99 x i32]* [[ARRS]], i64 0, i64 42
  // CHECK: [[TV5:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: MAP.TOFROM:AGGRHEAD"([99 x i32]* [[ARRS]], i32* [[AI14]], i64 80)
  // CHECK: region.exit(token [[TV5]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(arrS[42:20])
  {
    arrS[50] = 3;
  }

  // CHECK: [[AI16:%.+]] = getelementptr inbounds [49 x [29 x i32]], [49 x [29 x i32]]* [[ARRD]], i64 0, i64 9
  // CHECK: [[TV6:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: MAP.TOFROM:AGGRHEAD"([49 x [29 x i32]]* [[ARRD]], [29 x i32]* [[AI16]], i64 1392)
  // CHECK: region.exit(token [[TV6]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(arrD[9:12][:])
  {
    arrD[11][14] = 4;
  }
}
