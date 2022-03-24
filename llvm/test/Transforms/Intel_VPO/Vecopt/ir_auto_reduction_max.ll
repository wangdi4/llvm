; RUN: opt -S -vplan-vec -disable-vplan-predicator -vplan-force-vf=4 < %s  -instcombine | FileCheck %s

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
; CHECK:         [[TMP0:%.*]] = load i32, i32* %A, align 4
; CHECK:       VPlannedBB2:
; CHECK-NEXT:    [[BROADCAST_SPLATINSERT0:%.*]] = insertelement <4 x i32> poison, i32 [[TMP0]], i64 0
; CHECK-NEXT:    [[BROADCAST_SPLAT0:%.*]] = shufflevector <4 x i32> [[BROADCAST_SPLATINSERT0]], <4 x i32> poison, <4 x i32> zeroinitializer
; CHECK:       vector.body:
; CHECK:         [[VEC_PHI50:%.*]] = phi <4 x i32> [ [[BROADCAST_SPLAT0]], %VPlannedBB2 ], [ [[TMP6:%.*]], %vector.body ]
; CHECK:         [[TMP6]] = call <4 x i32> @llvm.smax.v4i32(<4 x i32> [[WIDE_LOAD0:%.*]], <4 x i32> [[VEC_PHI50]])
; CHECK:       VPlannedBB6:
; CHECK-NEXT:    [[TMP9:%.*]] = call i32 @llvm.vector.reduce.smax.v4i32(<4 x i32> [[TMP6]])
; CHECK:       final.merge:
; CHECK-NEXT:    [[UNI_PHI120:%.*]] = phi i32 [ [[TMP13:%.*]], [[VPLANNEDBB110:%.*]] ], [ [[TMP9]], {{.*}} ]
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
