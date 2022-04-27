; RUN: opt -hir-ssa-deconstruction -analyze -hir-framework -enable-new-pm=0 < %s  2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -disable-output  < %s  2>&1 | FileCheck %s

; Check that we recognize the llvm.loop.intel.max.trip_count metadata.
; The metadata is supposed to be recognized as legal max tripcount under the contract with frontend.
; Notice that the smaller of llvm.loop.intel.loopcount_maximum and llvm.loop.intel.max.trip_count is
; regarded as legal max trip count.

; CHECK: DO i1
; CHECK-SAME: <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 5> <min_trip_count = 4> <avg_trip_count = 7> <max_trip_count = 10>


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(float* nocapture %A, float* nocapture readonly %B, i32 %n) {
entry:
  %cmp.7 = icmp sgt i32 %n, 0
  br i1 %cmp.7, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds float, float* %B, i64 %indvars.iv
  %0 = bitcast float* %arrayidx to i32*
  %1 = load i32, i32* %0, align 4
  %arrayidx2 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %2 = bitcast float* %arrayidx2 to i32*
  store i32 %1, i32* %2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body, !llvm.loop !0

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

!0 = distinct !{!0, !1, !2, !3, !4}
!1 = !{!"llvm.loop.intel.loopcount_minimum", i32 4}
!2 = !{!"llvm.loop.intel.loopcount_maximum", i32 10}
!3 = !{!"llvm.loop.intel.loopcount_average", i32 7}
!4 = !{!"llvm.loop.intel.max.trip_count", i32 5}
