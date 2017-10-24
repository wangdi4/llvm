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
; CHECK: vector.ph
; CHECK: %[[TMP0:.*]] = insertelement <4 x i32> <i32 undef, i32 0, i32 0, i32 0>, i32 %Init, i32 0
; CHECK: vector.body
; CHECK: %vec.phi = phi <4 x i32> [ %[[TMP0]], %vector.ph ], [ {{.*}}, %vector.body ]
; CHECK: %wide.load = load <4 x i32>
; CHECK: add nsw <4 x i32> %wide.load, %vec.phi
; CHECK: middle.block
; CHECK: shufflevector <4 x i32>
; CHECK: add <4 x i32>
; CHECK: shufflevector <4 x i32>
; CHECK: extractelement <4 x i32>

define i32 @foo(i32* nocapture readonly %A, i32 %N, i32 %Init) {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
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
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret i32 %Sum.0.lcssa

}
declare void @llvm.intel.directive(metadata)
