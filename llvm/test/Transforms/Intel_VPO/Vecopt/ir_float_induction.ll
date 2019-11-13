; RUN: opt -S -VPlanDriver -enable-vp-value-codegen=false -disable-vplan-predicator -disable-vplan-subregions -vplan-force-vf=4 < %s  -instcombine | FileCheck %s
; RUN: opt -S -VPlanDriver -enable-vp-value-codegen=true -disable-vplan-predicator -disable-vplan-subregions -vplan-force-vf=4 < %s  -instcombine | FileCheck --check-prefix VPBCG %s

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

define void @fp_iv_loop(float %init, float* noalias nocapture %A, i32 %N) local_unnamed_addr #0 {
; CHECK-LABEL: @fp_iv_loop(
; CHECK:       vector.ph:
; CHECK:    [[DOTSPLATINSERT:%.*]] = insertelement <4 x float> undef, float [[INIT:%.*]], i32 0
; CHECK:    [[DOTSPLAT:%.*]] = shufflevector <4 x float> [[DOTSPLATINSERT]], <4 x float> undef, <4 x i32> zeroinitializer
; CHECK:    [[DOTSPLATINSERT2:%.*]] = insertelement <4 x float> undef, float [[TMP0:%.*]], i32 0
; CHECK:    [[DOTSPLAT3:%.*]] = shufflevector <4 x float> [[DOTSPLATINSERT2]], <4 x float> undef, <4 x i32> zeroinitializer
; CHECK:    [[TMP2:%.*]] = fmul fast <4 x float> [[DOTSPLAT3]], <float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00>
; CHECK:    [[TMP3:%.*]] = fmul fast float [[TMP0]], 4.000000e+00
; CHECK:    [[DOTSPLATINSERT4:%.*]] = insertelement <4 x float> undef, float [[TMP3]], i32 0
; CHECK:    [[DOTSPLAT5:%.*]] = shufflevector <4 x float> [[DOTSPLATINSERT4]], <4 x float> undef, <4 x i32> zeroinitializer
; CHECK:       vector.body:
; CHECK:    [[INDEX:%.*]] = phi i64 [ 0,
; CHECK:    [[VEC_IND6:%.*]] = phi <4 x float>
; CHECK:    store <4 x float> [[VEC_IND6]]
; CHECK:    [[VEC_IND_NEXT7:%.*]] = fsub fast <4 x float> [[VEC_IND6]], [[DOTSPLAT5]]
;
; VPBCG-LABEL: @fp_iv_loop(
; VPBCG:       for.body.lr.ph:
; VPBCG:    [[TMP0:%.*]] = load float, float* @fp_inc, align 4
; VPBCG:       vector.ph:
; VPBCG:    [[INITIND_START_BCAST_SPLATINSERT:%.*]] = insertelement <4 x float> undef, float [[INIT:%.*]], i32 0
; VPBCG:    [[INITIND_START_BCAST_SPLAT:%.*]] = shufflevector <4 x float> [[INITIND_START_BCAST_SPLATINSERT]], <4 x float> undef, <4 x i32> zeroinitializer
; VPBCG:    [[IND_STEP_VEC_SPLATINSERT:%.*]] = insertelement <4 x float> undef, float [[TMP0]], i32 0
; VPBCG:    [[IND_STEP_VEC_SPLAT:%.*]] = shufflevector <4 x float> [[IND_STEP_VEC_SPLATINSERT]], <4 x float> undef, <4 x i32> zeroinitializer
; VPBCG:    [[TMP1:%.*]] = fmul fast <4 x float> [[IND_STEP_VEC_SPLAT]], <float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00>
; VPBCG:    [[TMP2:%.*]] = fsub <4 x float> [[INITIND_START_BCAST_SPLAT]], [[TMP1]]
; VPBCG:    [[TMP3:%.*]] = fmul float [[TMP0]], 4.000000e+00
; VPBCG:    [[IND_STEP_INIT_SPLATINSERT:%.*]] = insertelement <4 x float> undef, float [[TMP3]], i32 0
; VPBCG:    [[IND_STEP_INIT_SPLAT:%.*]] = shufflevector <4 x float> [[IND_STEP_INIT_SPLATINSERT]], <4 x float> undef, <4 x i32> zeroinitializer
; VPBCG:       vector.body:
; VPBCG:    [[VEC_PHI2:%.*]] = phi <4 x float> [ [[TMP2]], [[VECTOR_PH:%.*]] ], [ [[TMP4:%.*]], [[VECTOR_BODY:%.*]] ]
; VPBCG:    [[TMP4]] = fsub fast <4 x float> [[VEC_PHI2]], [[IND_STEP_INIT_SPLAT]]
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
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
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
