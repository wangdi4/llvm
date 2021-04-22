// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// Verify nontemporal clause behaviors for  pointers, refs, arrays, vlas.
//
void foo()
{
#define N 10

  int i, a[N] = {0};
  int (&ra)[N] = a;

  // simd nontemporal(<array>)
  // CHECK: DIR.OMP.SIMD
  // CHECK-SAME: QUAL.OMP.NONTEMPORAL
  // CHECK: DIR.OMP.END.SIMD
#pragma omp simd nontemporal(a)
  for (i=0; i<N; i++) {
    a[i] = 1;
  }

  // simd nontemporal(<reference to array>)
  // CHECK: DIR.OMP.SIMD
  // CHECK-SAME: QUAL.OMP.NONTEMPORAL
  // CHECK: DIR.OMP.END.SIMD
#pragma omp simd nontemporal(ra)
  for (i=0; i<N; i++) {
    ra[i] = i;
  }
}

// Verify the nontemporal address is used in NONTEMPORAL.
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
  // CHECK-SAME: "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(float** [[FPLOC]]
  // CHECK-SAME: "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(float** [[FPADDR]])
  // CHECK-SAME: "QUAL.OMP.NONTEMPORAL"([10 x float]* [[ARR]])
  // CHECK-SAME: "QUAL.OMP.NONTEMPORAL"(float* [[VLA]])
  // CHECK-SAME: "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(float** [[LFPR]])
  // CHECK-SAME: "QUAL.OMP.NONTEMPORAL"([10 x float]* [[LARR]])
  // CHECK: DIR.OMP.END.SIMD
  #pragma omp simd nontemporal(FP_local,FP) \
                   nontemporal(ARR)         \
                   nontemporal(VLA)         \
                   nontemporal(FP_ref)      \
                   nontemporal(ARR_ref)
  for (int I = 0; I < M; ++I) {
    VLA[I] = I;
    FP[I] = I;
    ARR[I] = I;
  }
  return VLA[M/2] + FP[5] + ARR[5];
}
// end INTEL_COLLAB
