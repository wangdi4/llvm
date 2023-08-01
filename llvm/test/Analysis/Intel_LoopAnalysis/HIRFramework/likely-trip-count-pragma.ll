; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -disable-output < %s 2>&1 | FileCheck %s

; Verify that we recognize likely trip counts specified using loopcount pragma.
; CHECK: DO i1
; CHECK-SAME: <trip_counts = 4, 3, 5>


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(ptr nocapture %A, ptr nocapture readonly %B, i32 %n) {
entry:
  %cmp.7 = icmp sgt i32 %n, 0
  br i1 %cmp.7, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds float, ptr %B, i64 %indvars.iv
  %0 = bitcast ptr %arrayidx to ptr
  %1 = load i32, ptr %0, align 4
  %arrayidx2 = getelementptr inbounds float, ptr %A, i64 %indvars.iv
  %2 = bitcast ptr %arrayidx2 to ptr
  store i32 %1, ptr %2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body, !llvm.loop !0

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.intel.loopcount", i32 4, i32 3, i32 5}
