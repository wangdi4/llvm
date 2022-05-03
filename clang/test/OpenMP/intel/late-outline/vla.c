// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// Checks that VLA size expressions are local to regions.
//CHECK-LABEL: vla_test
void vla_test(int k, int nz)
{
  //CHECK: [[KADDR:%k.addr.*]] = alloca i32, align 4
  //CHECK: [[NZADDR:%nz.addr.*]] = alloca i32, align 4
  //CHECK: [[VLATMP1:%omp.vla.tmp.*]] = alloca i64,
  //CHECK: [[VLATMP2:%omp.vla.tmp.*]] = alloca i64,
  int rhsX[5][nz];

  //CHECK: [[L0:%[0-9]+]] = load i32, ptr [[NZADDR]], align 4
  //CHECK-NEXT: [[L1:%[0-9]+]] = zext i32 [[L0]] to i64
  //CHECK: [[L3:%[0-9]+]] =  mul nuw i64 5, [[L1]]
  //CHECK: [[VLA:%vla.*]] = alloca i32, i64 [[L3]], align 16
  //CHECK: [[L4:%[0-9]+]] = mul nsw i64 3, [[L1]]
  //CHECK: [[AI0:%arrayidx.*]] = getelementptr inbounds i32, ptr [[VLA]], i64 [[L4]]
  //CHECK: [[L5:%[0-9]+]] = load i32, ptr [[KADDR]], align 4
  //CHECK: [[IP:%idxprom.*]] = sext i32 [[L5]] to i64
  //CHECK: [[AI1:%arrayidx.*]] = getelementptr inbounds i32, ptr [[AI0]], i64 [[IP]]
  //CHECK: store i32 73, ptr [[AI1]], align 4
  rhsX[3][k] = 73;

  //CHECK: store i64 [[L1]], ptr [[VLATMP1]], align 8
  //CHECK: region.entry{{.*}}DIR.OMP.PARALLEL
  //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[VLATMP1]]
  #pragma omp parallel
  {
  //CHECK: [[AL1:%[0-9]+]] = load i64, ptr [[VLATMP1]], align 8
  //CHECK: [[L3:%[0-9]+]] =  mul nsw i64 0, [[AL1]]
  //CHECK: [[AI0:%arrayidx.*]] = getelementptr inbounds i32, ptr [[VLA]], i64 [[L3]]
  //CHECK: [[L5:%[0-9]+]] = load i32, ptr [[KADDR]], align 4
  //CHECK: [[IP:%idxprom.*]] = sext i32 [[L5]] to i64
  //CHECK: [[AI1:%arrayidx.*]] = getelementptr inbounds i32, ptr [[AI0]], i64 [[IP]]
  //CHECK: store i32 70, ptr [[AI1]], align 4
    rhsX[0][k] = 70;
  }
  //CHECK: region.exit{{.*}}DIR.OMP.END.PARALLEL
  //CHECK: store i64 [[L1]], ptr [[VLATMP2]], align 8
  //CHECK: region.entry{{.*}}DIR.OMP.PARALLEL
  //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[VLATMP2]]
  #pragma omp parallel
  {
  //CHECK: [[BL1:%[0-9]+]] = load i64, ptr [[VLATMP2]], align 8
  //CHECK: [[L3:%[0-9]+]] =  mul nsw i64 1, [[BL1]]
  //CHECK: [[AI0:%arrayidx.*]] = getelementptr inbounds i32, ptr [[VLA]], i64 [[L3]]
  //CHECK: [[L5:%[0-9]+]] = load i32, ptr [[KADDR]], align 4
  //CHECK: [[IP:%idxprom.*]] = sext i32 [[L5]] to i64
  //CHECK: [[AI1:%arrayidx.*]] = getelementptr inbounds i32, ptr [[AI0]], i64 [[IP]]
  //CHECK: store i32 71, ptr [[AI1]], align 4
    rhsX[1][k] = 71;
  }
  //CHECK: region.exit{{.*}}DIR.OMP.END.PARALLEL

  //CHECK: [[L4:%[0-9]+]] = mul nsw i64 2, [[L1]]
  //CHECK: [[AI0:%arrayidx.*]] = getelementptr inbounds i32, ptr [[VLA]], i64 [[L4]]
  //CHECK: [[L5:%[0-9]+]] = load i32, ptr [[KADDR]], align 4
  //CHECK: [[IP:%idxprom.*]] = sext i32 [[L5]] to i64
  //CHECK: [[AI1:%arrayidx.*]] = getelementptr inbounds i32, ptr [[AI0]], i64 [[IP]]
  //CHECK: store i32 72, ptr [[AI1]], align 4
  rhsX[2][k] = 72;
}

//CHECK-LABEL: vla_test_two
void vla_test_two(int m)
{
  int i;
  //CHECK: "DIR.OMP.PARALLEL.LOOP"()
  #pragma omp parallel for
  for (i = 0; i < m; i++) {
    int d[m][m];
    //CHECK: [[VLA:%vla.*]] = alloca i32, i64
    d[i][i] = i*3;
    //CHECK: DIR.OMP.PARALLEL
    //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[VLA]], i32 0,
    #pragma omp parallel
    {
           d[3][i] = i;
    }
    //CHECK: DIR.OMP.END.PARALLEL
  }
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"()
}

//CHECK-LABEL: vla_test_three
void vla_test_three(int x, int const size) {
  //CHECK: [[VLATMP:%omp.vla.tmp.*]] = alloca i64,
  switch (x) {
  case 1: {
    float a[5][size];
    a[1][4] = 4;
    break;
  }
  case 2: {
    float a[5][size];
    //CHECK: store i64 [[V:%[0-9]+]], ptr [[VLATMP]],
    //CHECK: "DIR.OMP.PARALLEL"
    //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[VLATMP]]
    #pragma omp parallel
    //CHECK: [[L:%[0-9]+]] = load i64, ptr [[VLATMP]]
    //CHECK: [[L]]
    //CHECK: "DIR.OMP.END.PARALLEL"
    for (int i = 0; i < size; i++)
      a[1][i] = 4;
    break;
  }
  }
}

//CHECK-LABEL: vla_test_four
void vla_test_four(int x, int const size) {
  //CHECK: [[VLATMP:%omp.vla.tmp.*]] = alloca i64,
  switch (x) {
  case 1: {
    float a[5][size];
    a[1][4] = 4;
    break;
  }
  case 2: {
    float a[5][size];
    //CHECK: store i64 [[V:%[0-9]+]], ptr [[VLATMP]],
    //CHECK: "DIR.OMP.TARGET"
    //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[VLATMP]]
    //CHECK: "DIR.OMP.TEAMS"
    //CHECK: "DIR.OMP.DISTRIBUTE.PARLOOP"
    #pragma omp target teams distribute parallel for
    //CHECK: [[L:%[0-9]+]] = load i64, ptr [[VLATMP]]
    //CHECK: [[L]]
    //CHECK: "DIR.OMP.END.DISTRIBUTE.PARLOOP"
    //CHECK: "DIR.OMP.END.TEAMS"
    //CHECK: "DIR.OMP.END.TARGET"
    for (int i = 0; i < size; i++)
      a[1][i] = 4;
    break;
  }
  }
}

//CHECK-LABEL: vla_test_five
void vla_test_five(int x, int const size) {
  //CHECK: [[VLATMP1:%omp.vla.tmp.*]] = alloca i64,
  //CHECK: [[VLATMP2:%omp.vla.tmp.*]] = alloca i64,
  //CHECK: [[VLATMP3:%omp.vla.tmp.*]] = alloca i64,
  float a[5][size];

  //CHECK: store i64 [[V1:%[0-9]+]], ptr [[VLATMP1]],
  //CHECK: "DIR.OMP.PARALLEL"
  #pragma omp parallel
  {
    //CHECK: [[L1:%[0-9]+]] = load i64, ptr [[VLATMP1]]
    //CHECK: mul{{.*}}[[L1]]
    a[0][3] = x;

    //CHECK: store i64 [[L1]], ptr [[VLATMP2]],
    //CHECK: "DIR.OMP.PARALLEL"
    #pragma omp parallel
    {
      //CHECK: [[L2:%[0-9]+]] = load i64, ptr [[VLATMP2]]
      //CHECK: mul{{.*}}[[L2]]
      a[1][3] = x;
    }
    //CHECK: "DIR.OMP.END.PARALLEL"
    //CHECK: store i64 [[L1]], ptr [[VLATMP3]],
    //CHECK: "DIR.OMP.PARALLEL"
    #pragma omp parallel
    {
      //CHECK: [[L3:%[0-9]+]] = load i64, ptr [[VLATMP3]]
      //CHECK: mul{{.*}}[[L3]]
      a[2][3] = x;
    }
    //CHECK: "DIR.OMP.END.PARALLEL"
    //CHECK: mul{{.*}}[[L1]]
    a[3][3] = x;
  }
  //CHECK: "DIR.OMP.END.PARALLEL"
  //CHECK: mul{{.*}}[[V1]]
  a[4][4] = x;
}

// end INTEL_COLLAB
