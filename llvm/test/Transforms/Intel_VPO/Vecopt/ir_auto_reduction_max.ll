; RUN: opt -VPlanDriver -enable-vp-value-codegen=false -disable-vplan-predicator -disable-vplan-subregions -vplan-force-vf=4 -S < %s  -instcombine | FileCheck %s
; RUN: opt -VPlanDriver -enable-vp-value-codegen=true -disable-vplan-predicator -disable-vplan-subregions -vplan-force-vf=4 -S < %s  -instcombine | FileCheck --check-prefix VPBCG %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;int foo(int *A, int N, int Init) {
;  int Sum = Init;
;
;#pragma opt simd
;  for (int i=0; i<N; i++)
;    Sum+= A[i];
;  return Sum;
;}

define i32 @foo(i32* nocapture readonly %A, i32 %N) {
; CHECK-LABEL: @foo(
; CHECK:       [[TMP0:%.*]] = load i32, i32* [[A:%.*]]
; CHECK:       vector.ph:
; CHECK:       [[MINMAX_IDENT_SPLATINSERT:%.*]] = insertelement <4 x i32> undef, i32 [[TMP0]], i32 0
; CHECK:       [[MINMAX_IDENT_SPLAT:%.*]] = shufflevector <4 x i32> [[MINMAX_IDENT_SPLATINSERT]], <4 x i32> undef, <4 x i32> zeroinitializer
; CHECK:       vector.body:
; CHECK:       [[VEC_PHI:%.*]] = phi <4 x i32> [ [[MINMAX_IDENT_SPLAT]], [[VECTOR_PH:%.*]] ], [ [[TMP4:%.*]], [[VECTOR_BODY:%.*]] ]
; CHECK:       [[WIDE_LOAD:%.*]] = load <4 x i32>, <4 x i32>* [[TMP2:%.*]], align 4
; CHECK:       [[TMP3:%.*]] = icmp sgt <4 x i32> [[WIDE_LOAD]], [[VEC_PHI]]
; CHECK:       [[TMP4]] = select <4 x i1> [[TMP3]], <4 x i32> [[WIDE_LOAD]], <4 x i32> [[VEC_PHI]]
; CHECK:       middle.block:
; CHECK-NEXT:    [[RDX_SHUF:%.*]] = shufflevector <4 x i32> [[TMP4]], <4 x i32> undef, <4 x i32> <i32 2, i32 3, i32 undef, i32 undef>
; CHECK-NEXT:    [[RDX_MINMAX_CMP:%.*]] = icmp sgt <4 x i32> [[TMP4]], [[RDX_SHUF]]
; CHECK-NEXT:    [[RDX_MINMAX_SELECT:%.*]] = select <4 x i1> [[RDX_MINMAX_CMP]], <4 x i32> [[TMP4]], <4 x i32> [[RDX_SHUF]]
; CHECK-NEXT:    [[RDX_SHUF3:%.*]] = shufflevector <4 x i32> [[RDX_MINMAX_SELECT]], <4 x i32> undef, <4 x i32> <i32 1, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:    [[RDX_MINMAX_CMP4:%.*]] = icmp sgt <4 x i32> [[RDX_MINMAX_SELECT]], [[RDX_SHUF3]]
; CHECK-NEXT:    [[RDX_MINMAX_SELECT5:%.*]] = select <4 x i1> [[RDX_MINMAX_CMP4]], <4 x i32> [[RDX_MINMAX_SELECT]], <4 x i32> [[RDX_SHUF3]]
; CHECK-NEXT:    [[TMP6:%.*]] = extractelement <4 x i32> [[RDX_MINMAX_SELECT5]], i32 0
;
; VPBCG-LABEL: @foo(
; VPBCG:       [[TMP0:%.*]] = load i32, i32* [[A:%.*]], align 4
; VPBCG:       vector.ph:
; VPBCG:       [[BROADCAST_SPLATINSERT:%.*]] = insertelement <4 x i32> undef, i32 [[TMP0]], i32 0
; VPBCG:       [[BROADCAST_SPLAT:%.*]] = shufflevector <4 x i32> [[BROADCAST_SPLATINSERT]], <4 x i32> undef, <4 x i32> zeroinitializer
; VPBCG:       vector.body:
; VPBCG:       [[VEC_PHI1:%.*]] = phi <4 x i32> [ [[BROADCAST_SPLAT]], [[VECTOR_PH:%.*]] ], [ [[TMP3:%.*]], [[VECTOR_BODY:%.*]] ]
; VPBCG:       [[TMP2:%.*]] = icmp sgt <4 x i32> [[WIDE_MASKED_GATHER:%.*]], [[VEC_PHI1]]
; VPBCG:       [[TMP3]] = select <4 x i1> [[TMP2]], <4 x i32> [[WIDE_MASKED_GATHER]], <4 x i32> [[VEC_PHI1]]
; VPBCG:       VPlannedBB:
; VPBCG:       [[TMP6:%.*]] = call i32 @llvm.experimental.vector.reduce.smax.v4i32(<4 x i32> [[TMP3]])
; VPBCG:       scalar.ph:
; VPBCG:       [[BC_MERGE_REDUCTION:%.*]] = phi i32 [ [[TMP0]], [[FOR_BODY_PH:%.*]] ], [ [[TMP0]], [[MIN_ITERS_CHECKED:%.*]] ], [ [[TMP6]], [[MIDDLE_BLOCK:%.*]] ]
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  %0 = load i32, i32* %A, align 4
  %cmp13 = icmp sgt i32 %N, 1
  br i1 %cmp13, label %for.body.ph, label %for.cond.cleanup

for.body.ph:                                 ; preds = %0
  %wide.trip.count = zext i32 %N to i64
  br label %for.body

for.body:                                           ; preds = %for.body.ph, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 1, %for.body.ph ]
  %Max.014 = phi i32 [ %.Max.0, %for.body ], [ %0, %for.body.ph ]
  %arrayidx1 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx1, align 4
  %cmp2 = icmp sgt i32 %1, %Max.014
  %.Max.0 = select i1 %cmp2, i32 %1, i32 %Max.014
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:                             ; preds = %for.body
  %.Max.0.lcssa = phi i32 [ %.Max.0, %for.body ]
  br label %for.cond.cleanup

for.cond.cleanup:                                      ; preds = %for.cond.cleanup.loopexit, %0
  %Max.0.lcssa = phi i32 [ %0, %DIR.QUAL.LIST.END.2 ], [ %.Max.0.lcssa, %for.cond.cleanup.loopexit ]
  br label %end.simd

end.simd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret i32 %Max.0.lcssa
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
; CCCC-LABEL: foo
; CCCC: %[[TMP0:.*]] = load i32, i32* %A
; CCCC: vector.ph
; CCCC: insertelement <4 x i32> undef, i32 %[[TMP0]], i32 0
; CCCC: %[[TMP1:.*]] = shufflevector <4 x i32> {{.*}} zeroinitializer
; CCCC: vector.body
; CCCC: %vec.phi = phi <4 x i32> [ %[[TMP1]], %vector.ph ], [ {{.*}}, %vector.body ]
; CCCC: %wide.load = load <4 x i32>
; CCCC: %[[TMP2:.*]] = icmp sgt <4 x i32> %wide.load, %vec.phi
; CCCC: select <4 x i1> %[[TMP2]], <4 x i32> %wide.load, <4 x i32> %vec.phi
; CCCC: middle.block
; CCCC: shufflevector <4 x i32>
; CCCC: icmp sgt <4 x i32>
; CCCC: select <4 x i1>
; CCCC: shufflevector <4 x i32>
; CCCC: icmp sgt <4 x i32>

