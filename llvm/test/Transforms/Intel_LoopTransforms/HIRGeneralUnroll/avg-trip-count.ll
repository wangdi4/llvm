; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll,print<hir>" -disable-output -debug-only=hir-general-unroll < %s  2>&1 | FileCheck %s

; Check that we skip unroll if avg trip count is less than the threshold.

; CHECK: Skipping unroll of small trip count loop!

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000> <nounroll> <avg_trip_count = 7>
; CHECK: |   %ld = (%B)[i1];
; CHECK: |   (@A)[0][i1] = %ld;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(ptr nocapture readonly %B, i64 %n) {
entry:
  %cmp.7 = icmp sgt i64 %n, 0
  br i1 %cmp.7, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds float, ptr %B, i64 %indvars.iv
  %ld = load float, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [1000 x float], ptr @A, i64 0, i64 %indvars.iv
  store float %ld, ptr %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64  %indvars.iv.next, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body, !llvm.loop !0

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.intel.loopcount_average", i32 7}
