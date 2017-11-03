; RUN: opt -S -VPlanDriver -disable-vplan-predicator -disable-vplan-subregions < %s  -instcombine | FileCheck %s

;float fp_inc;
;void fp_iv_loop(float init, float * __restrict__ A, int N) {
;  float x = init;
;  for (int i=0; i < N; ++i) {
;    A[i] = x;
;    x -= fp_inc;
;  }
;}

@fp_inc = local_unnamed_addr global float 0.000000e+00, align 4

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: fp_iv_loop
; CHECK: min.iters.checked
; CHECK: vector.ph:
; CHECK:  %[[TMP0:.*]] = insertelement <4 x float> undef, float %0, i32 0
; CHECK:  %[[TMP1:.*]] = shufflevector <4 x float> %[[TMP0]], <4 x float> undef, <4 x i32> zeroinitializer
; CHECK:  %[[TMP2:.*]] = fmul fast <4 x float> %[[TMP1]], <float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00>
; CHECK: %[[TMP3:.*]] = fmul fast float %0, 4.000000e+00
; CHECK: %[[TMP4:.*]] = insertelement <4 x float> undef, float %[[TMP3]], i32 0
; CHECK: %[[TMP5:.*]] = shufflevector <4 x float> %[[TMP4]], <4 x float> undef, <4 x i32> zeroinitializer
; CHECK: vector.body
; CHECK: %index = phi i64 [ 0,
; CHECK: %[[VEC_IND:.*]] = phi <4 x float>
; CHECK: store <4 x float> %[[VEC_IND]]
; CHECK: fsub fast <4 x float> %[[VEC_IND]], %[[TMP5]]

define void @fp_iv_loop(float %init, float* noalias nocapture %A, i32 %N) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  %cmp6 = icmp sgt i32 %N, 0
  br i1 %cmp6, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %DIR.QUAL.LIST.END.2
  %0 = load float, float* @fp_inc, align 4
  %wide.trip.count = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %x.07 = phi float [ %init, %for.body.lr.ph ], [ %sub, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %A, i64 %indvars.iv
  store float %x.07, float* %arrayidx, align 4
  %sub = fsub fast float %x.07, %0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %DIR.QUAL.LIST.END.2
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret void
}

declare void @llvm.intel.directive(metadata)
declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32)

