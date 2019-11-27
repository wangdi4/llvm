; RUN: opt -VPlanDriver -enable-vp-value-codegen=false -disable-vplan-predicator -disable-vplan-subregions -S -vplan-force-vf=4 < %s  -instcombine | FileCheck %s
; RUN: opt -VPlanDriver -enable-vp-value-codegen=true -disable-vplan-predicator -disable-vplan-subregions -S -vplan-force-vf=4 < %s  -instcombine | FileCheck --check-prefix VPBCG %s

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

define i32 @foo(i32* nocapture readonly %A, i32 %N, i32 %Init) {
; CHECK:         vector.ph:
; CHECK-NEXT:    [[TMP0:%.*]] = insertelement <4 x i32> <i32 undef, i32 0, i32 0, i32 0>, i32 [[INIT:%.*]], i32 0
; CHECK:         vector.body:
; CHECK:         [[VEC_PHI:%.*]] = phi <4 x i32> [ [[TMP0]], [[VECTOR_PH:%.*]] ], [ [[TMP2:%.*]], [[VECTOR_BODY:%.*]] ]
; CHECK:         [[WIDE_LOAD:%.*]] = load <4 x i32>, <4 x i32>* [[TMP1:.*]], align 4
; CHECK-NEXT:    [[TMP2]] = add nsw <4 x i32> [[WIDE_LOAD]], [[VEC_PHI]]
; CHECK:       middle.block:
; CHECK-NEXT:    [[RDX_SHUF:%.*]] = shufflevector <4 x i32> [[TMP2]], <4 x i32> undef, <4 x i32> <i32 2, i32 3, i32 undef, i32 undef>
; CHECK-NEXT:    [[BIN_RDX:%.*]] = add <4 x i32> [[TMP2]], [[RDX_SHUF]]
; CHECK-NEXT:    [[RDX_SHUF3:%.*]] = shufflevector <4 x i32> [[BIN_RDX]], <4 x i32> undef, <4 x i32> <i32 1, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:    [[BIN_RDX4:%.*]] = add <4 x i32> [[BIN_RDX]], [[RDX_SHUF3]]
; CHECK-NEXT:    [[TMP4:%.*]] = extractelement <4 x i32> [[BIN_RDX4]], i32 0
;
; VPBCG:         vector.ph:
; VPBCG-NEXT:    [[RED_INIT_INSERT:%.*]] = insertelement <4 x i32> <i32 undef, i32 0, i32 0, i32 0>, i32 [[INIT:%.*]], i32 0
; VPBCG:         vector.body:
; VPBCG:         [[VEC_PHI1:%.*]] = phi <4 x i32> [ [[RED_INIT_INSERT]], [[VECTOR_PH:%.*]] ], [ [[TMP0:%.*]], [[VECTOR_BODY:%.*]] ]
; VPBCG:         [[TMP0]] = add nsw <4 x i32> [[WIDE_MASKED_GATHER:%.*]], [[VEC_PHI1]]
; VPBCG:         VPlannedBB:
; VPBCG-NEXT:    [[TMP3:%.*]] = call i32 @llvm.experimental.vector.reduce.add.v4i32(<4 x i32> [[TMP0]])
; VPBCG:         middle.block:
; VPBCG:         [[BC_MERGE_REDUCTION:%.*]] = phi i32 [ [[INIT:%.*]], [[FOR_BODY_PH:%.*]] ], [ [[TMP3]], [[MIDDLE_BLOCK:%.*]] ]
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  %cmp6 = icmp sgt i32 %N, 0
  br i1 %cmp6, label %for.body.ph, label %for.cond.cleanup

for.body.ph:                                 ; preds = %0
  %wide.trip.count = zext i32 %N to i64
  br label %for.body

for.body:                                           ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.ph ]
  %Sum.07 = phi i32 [ %add, %for.body ], [ %Init, %for.body.ph ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %A.i = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %A.i, %Sum.07
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:                             ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  br label %for.cond.cleanup

for.cond.cleanup:                                      ; preds = %for.cond.cleanup.loopexit, %0
  %Sum.0.lcssa = phi i32 [ %Init, %DIR.QUAL.LIST.END.2 ], [ %add.lcssa, %for.cond.cleanup.loopexit ]
  br label %end.simd

end.simd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret i32 %Sum.0.lcssa

}
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

