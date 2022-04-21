// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// Verify aligned clause accepts array/reference to array.
//
void foo()
{
#define N 10

  int i, a[N] = {0};
  int (&ra)[N] = a;

  // simd aligned(<array>)
  // CHECK: DIR.OMP.SIMD
  // CHECK-SAME: QUAL.OMP.ALIGNED
  // CHECK: DIR.OMP.END.SIMD
#pragma omp simd aligned(a)
  for (i=0; i<N; i++) {
    a[i] = 1;
  }

  // simd aligned(<reference to array>)
  // CHECK: DIR.OMP.SIMD
  // CHECK-SAME: QUAL.OMP.ALIGNED
  // CHECK: DIR.OMP.END.SIMD
#pragma omp simd aligned(ra)
  for (i=0; i<N; i++) {
    ra[i] = i;
  }
}

// Verify the aligned address is used in ALIGNED.
int bar(int M, float *FP) {

  // CHECK: [[FPADDR:%FP.addr]] = alloca float*, align 8
  // CHECK: [[FPLOC:%FP_local.*]] = alloca float*, align 8
  // CHECK: [[FPREF:%FP_ref.*]] = alloca float**, align 8
  // CHECK: [[ARR:%ARR.*]] = alloca [10 x float], align 16
  // CHECK: [[AREF:%ARR_ref.*]] = alloca [10 x float]*, align 8
  // CHECK: [[VLA:%vla.*]] = alloca float, i64 %{{[0-9]+}}, align 16
  float *FP_local = FP;
  float *&FP_ref = FP_local;
  float ARR[10];
  float (&ARR_ref)[10] = ARR;
  float VLA[M];

  // CHECK: [[LFPR:%[0-9]+]] = load float**, float*** [[FPREF]], align 8
  // CHECK: [[LARR:%[0-9]+]] = load [10 x float]*, [10 x float]** [[AREF]],
  //
  // CHECK: DIR.OMP.SIMD
  // CHECK-SAME: "QUAL.OMP.ALIGNED:PTR_TO_PTR"(float** [[FPLOC]], i32 32)
  // CHECK-SAME: "QUAL.OMP.ALIGNED:PTR_TO_PTR"(float** [[FPADDR]], i32 32)
  // CHECK-SAME: "QUAL.OMP.ALIGNED"([10 x float]* [[ARR]], i32 32)
  // CHECK-SAME: "QUAL.OMP.ALIGNED"(float* [[VLA]], i32 32)
  // CHECK-SAME: "QUAL.OMP.ALIGNED:PTR_TO_PTR"(float** [[LFPR]], i32 32)
  // CHECK-SAME: "QUAL.OMP.ALIGNED"([10 x float]* [[LARR]], i32 32)
  // CHECK: DIR.OMP.END.SIMD
  #pragma omp simd aligned(FP_local,FP:32) \
                   aligned(ARR:32)         \
                   aligned(VLA:32)         \
                   aligned(FP_ref:32)      \
                   aligned(ARR_ref:32)
  for (int I = 0; I < M; ++I) {
    VLA[I] = I;
    FP[I] = I;
    ARR[I] = I;
  }
  return VLA[M/2] + FP[5] + ARR[5];
}
// end INTEL_COLLAB
