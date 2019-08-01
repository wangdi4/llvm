// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
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

  //CHECK: [[L0:%[0-9]+]] = load i32, i32* [[NZADDR]], align 4
  //CHECK-NEXT: [[L1:%[0-9]+]] = zext i32 [[L0]] to i64
  //CHECK: [[L3:%[0-9]+]] =  mul nuw i64 5, [[L1]]
  //CHECK: [[VLA:%vla.*]] = alloca i32, i64 [[L3]], align 16
  //CHECK: [[L4:%[0-9]+]] = mul nsw i64 3, [[L1]]
  //CHECK: [[AI0:%arrayidx.*]] = getelementptr inbounds i32, i32* [[VLA]], i64 [[L4]]
  //CHECK: [[L5:%[0-9]+]] = load i32, i32* [[KADDR]], align 4
  //CHECK: [[IP:%idxprom.*]] = sext i32 [[L5]] to i64
  //CHECK: [[AI1:%arrayidx.*]] = getelementptr inbounds i32, i32* [[AI0]], i64 [[IP]]
  //CHECK: store i32 73, i32* [[AI1]], align 4
  rhsX[3][k] = 73;

  //CHECK: store i64 [[L1]], i64* [[VLATMP1]], align 8
  //CHECK: region.entry{{.*}}DIR.OMP.PARALLEL
  //CHECK-SAME: "QUAL.OMP.SHARED"(i64* [[VLATMP1]])
  #pragma omp parallel
  {
  //CHECK: [[AL1:%[0-9]+]] = load i64, i64* [[VLATMP1]], align 8
  //CHECK: [[L3:%[0-9]+]] =  mul nsw i64 0, [[AL1]]
  //CHECK: [[AI0:%arrayidx.*]] = getelementptr inbounds i32, i32* [[VLA]], i64 [[L3]]
  //CHECK: [[L5:%[0-9]+]] = load i32, i32* [[KADDR]], align 4
  //CHECK: [[IP:%idxprom.*]] = sext i32 [[L5]] to i64
  //CHECK: [[AI1:%arrayidx.*]] = getelementptr inbounds i32, i32* [[AI0]], i64 [[IP]]
  //CHECK: store i32 70, i32* [[AI1]], align 4
    rhsX[0][k] = 70;
  }
  //CHECK: region.exit{{.*}}DIR.OMP.END.PARALLEL
  //CHECK: store i64 [[L1]], i64* [[VLATMP2]], align 8
  //CHECK: region.entry{{.*}}DIR.OMP.PARALLEL
  //CHECK-SAME: "QUAL.OMP.SHARED"(i64* [[VLATMP2]])
  #pragma omp parallel
  {
  //CHECK: [[BL1:%[0-9]+]] = load i64, i64* [[VLATMP2]], align 8
  //CHECK: [[L3:%[0-9]+]] =  mul nsw i64 1, [[BL1]]
  //CHECK: [[AI0:%arrayidx.*]] = getelementptr inbounds i32, i32* [[VLA]], i64 [[L3]]
  //CHECK: [[L5:%[0-9]+]] = load i32, i32* [[KADDR]], align 4
  //CHECK: [[IP:%idxprom.*]] = sext i32 [[L5]] to i64
  //CHECK: [[AI1:%arrayidx.*]] = getelementptr inbounds i32, i32* [[AI0]], i64 [[IP]]
  //CHECK: store i32 71, i32* [[AI1]], align 4
    rhsX[1][k] = 71;
  }
  //CHECK: region.exit{{.*}}DIR.OMP.END.PARALLEL

  //CHECK: [[L4:%[0-9]+]] = mul nsw i64 2, [[L1]]
  //CHECK: [[AI0:%arrayidx.*]] = getelementptr inbounds i32, i32* [[VLA]], i64 [[L4]]
  //CHECK: [[L5:%[0-9]+]] = load i32, i32* [[KADDR]], align 4
  //CHECK: [[IP:%idxprom.*]] = sext i32 [[L5]] to i64
  //CHECK: [[AI1:%arrayidx.*]] = getelementptr inbounds i32, i32* [[AI0]], i64 [[IP]]
  //CHECK: store i32 72, i32* [[AI1]], align 4
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
    //CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[VLA]])
    #pragma omp parallel
    {
           d[3][i] = i;
    }
    //CHECK: DIR.OMP.END.PARALLEL
  }
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"()
}
// end INTEL_COLLAB
