; RUN: opt -VPlanDriver -disable-vplan-predicator -disable-vplan-subregions -S < %s  -instcombine | FileCheck %s

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

; CHECK-LABEL: foo
; CHECK: %[[TMP0:.*]] = load i32, i32* %A
; CHECK: vector.ph
; CHECK: insertelement <4 x i32> undef, i32 %[[TMP0]], i32 0
; CHECK: %[[TMP1:.*]] = shufflevector <4 x i32> {{.*}} zeroinitializer
; CHECK: vector.body
; CHECK: %vec.phi = phi <4 x i32> [ %[[TMP1]], %vector.ph ], [ {{.*}}, %vector.body ]
; CHECK: %wide.load = load <4 x i32>
; CHECK: %[[TMP2:.*]] = icmp sgt <4 x i32> %wide.load, %vec.phi
; CHECK: select <4 x i1> %[[TMP2]], <4 x i32> %wide.load, <4 x i32> %vec.phi
; CHECK: middle.block
; CHECK: shufflevector <4 x i32>
; CHECK: icmp sgt <4 x i32>
; CHECK: select <4 x i1>
; CHECK: shufflevector <4 x i32>
; CHECK: icmp sgt <4 x i32>

define i32 @foo(i32* nocapture readonly %A, i32 %N) {

entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
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
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret i32 %Max.0.lcssa
}



declare void @llvm.intel.directive(metadata)
