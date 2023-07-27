; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -print-before=hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll -disable-output 2>&1 < %s | FileCheck %s

; Verify that propagation utility is able to propagate the linear definition
; %t.018 = i1 into its use in (%A)[%t.018] and eliminate the temp after complete
; unroll

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 299, 1   <DO_LOOP>
; CHECK: |   %t.018 = i1;
; CHECK: |
; CHECK: |   + DO i2 = 0, 3, 1   <DO_LOOP> <unroll = 4>
; CHECK: |   |   (%A)[%t.018] = i2;
; CHECK: |   |   %t.018 = (%B)[i2];
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK:      + DO i1 = 0, 299, 1   <DO_LOOP>
; CHECK-NEXT: |   (%A)[i1] = 0;
; CHECK-NEXT: |   %t.018 = (%B)[0];
; CHECK-NEXT: |   (%A)[%t.018] = 1;
; CHECK-NEXT: |   %t.018 = (%B)[1];
; CHECK-NEXT: |   (%A)[%t.018] = 2;
; CHECK-NEXT: |   %t.018 = (%B)[2];
; CHECK-NEXT: |   (%A)[%t.018] = 3;
; CHECK-NEXT: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree norecurse nosync nounwind uwtable
define dso_local void @foo(ptr nocapture noundef writeonly %A, ptr nocapture noundef readonly %B) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.end
  %i.019 = phi i32 [ 0, %entry ], [ %inc7, %for.end ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %t.018 = phi i32 [ %i.019, %for.cond1.preheader ], [ %1, %for.body3 ]
  %idxprom = sext i32 %t.018 to i64
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %idxprom
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %arrayidx, align 4
  %arrayidx5 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond.not, label %for.end, label %for.body3, !llvm.loop !0

for.end:                                          ; preds = %for.body3
  %inc7 = add nuw nsw i32 %i.019, 1
  %exitcond20.not = icmp eq i32 %inc7, 300
  br i1 %exitcond20.not, label %for.end8, label %for.cond1.preheader

for.end8:                                         ; preds = %for.end
  ret void
}

!0 = !{!0, !1}
!1 = !{!"llvm.loop.unroll.count", i32 4}
