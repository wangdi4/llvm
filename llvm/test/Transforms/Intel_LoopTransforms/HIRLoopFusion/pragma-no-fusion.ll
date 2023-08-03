; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -disable-output -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Check that the same loopnest will be fused in the first case (foo)
; and will not be fused in the second case (foo_no_fusion) because of "nofusion" pragma

; CHECK-LABEL: Function: foo
;
; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1
; CHECK: |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1
;        |   |   (%a)[i2][i1] = i1 + i2;
;        |   |   %2 = (%a)[i2][i1];
;        |   |   (%b)[i2][i1] = i2 + %2;
; CHECK: |   + END LOOP
; CHECK-NOT: DO i2
; CHECK: + END LOOP
; CHECK-NOT: DO i1
; CHECK: END REGION
;
; CHECK-LABEL: Function: foo_no_fusion
;
; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1
; CHECK: |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1
;        |   |   (%a)[i2][i1] = i1 + i2;
; CHECK: |   + END LOOP
;        |
;        |
; CHECK: |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1
; CHECK-SAME: <no fusion>
;        |   |   %2 = (%a)[i2][i1];
;        |   |   (%b)[i2][i1] = i2 + %2;
; CHECK: |   + END LOOP
; CHECK: + END LOOP
; CHECK-NOT: DO i1
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr noalias nocapture %a, ptr nocapture %b, i32 %n) {
entry:
  %cmp49 = icmp sgt i32 %n, 0
  br i1 %cmp49, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  %wide.trip.count56 = sext i32 %n to i64
  br label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup10
  %indvars.iv58 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next59, %for.cond.cleanup10 ]
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup10
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body4:                                        ; preds = %for.body4, %for.body4.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.body4 ]
  %0 = add nuw nsw i64 %indvars.iv, %indvars.iv58
  %arrayidx6 = getelementptr inbounds [10 x i32], ptr %a, i64 %indvars.iv, i64 %indvars.iv58
  %1 = trunc i64 %0 to i32
  store i32 %1, ptr %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.body11.preheader, label %for.body4

for.body11.preheader:                             ; preds = %for.body4
  br label %for.body11

for.cond.cleanup10:                               ; preds = %for.body11
  %indvars.iv.next59 = add nuw nsw i64 %indvars.iv58, 1
  %exitcond61 = icmp eq i64 %indvars.iv.next59, %wide.trip.count56
  br i1 %exitcond61, label %for.cond.cleanup.loopexit, label %for.body4.lr.ph

for.body11:                                       ; preds = %for.body11.preheader, %for.body11
  %indvars.iv54 = phi i64 [ %indvars.iv.next55, %for.body11 ], [ 0, %for.body11.preheader ]
  %arrayidx15 = getelementptr inbounds [10 x i32], ptr %a, i64 %indvars.iv54, i64 %indvars.iv58
  %2 = load i32, ptr %arrayidx15, align 4
  %3 = trunc i64 %indvars.iv54 to i32
  %add16 = add nsw i32 %2, %3
  %arrayidx20 = getelementptr inbounds [10 x i32], ptr %b, i64 %indvars.iv54, i64 %indvars.iv58
  store i32 %add16, ptr %arrayidx20, align 4
  %indvars.iv.next55 = add nuw nsw i64 %indvars.iv54, 1
  %exitcond57 = icmp eq i64 %indvars.iv.next55, %wide.trip.count56
  br i1 %exitcond57, label %for.cond.cleanup10, label %for.body11
}

define dso_local void @foo_no_fusion(ptr noalias nocapture %a, ptr nocapture %b, i32 %n) {
entry:
  %cmp49 = icmp sgt i32 %n, 0
  br i1 %cmp49, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  %wide.trip.count56 = sext i32 %n to i64
  br label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup10
  %indvars.iv58 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next59, %for.cond.cleanup10 ]
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup10
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body4:                                        ; preds = %for.body4, %for.body4.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.body4 ]
  %0 = add nuw nsw i64 %indvars.iv, %indvars.iv58
  %arrayidx6 = getelementptr inbounds [10 x i32], ptr %a, i64 %indvars.iv, i64 %indvars.iv58
  %1 = trunc i64 %0 to i32
  store i32 %1, ptr %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.body11.preheader, label %for.body4

for.body11.preheader:                             ; preds = %for.body4
  br label %for.body11

for.cond.cleanup10:                               ; preds = %for.body11
  %indvars.iv.next59 = add nuw nsw i64 %indvars.iv58, 1
  %exitcond61 = icmp eq i64 %indvars.iv.next59, %wide.trip.count56
  br i1 %exitcond61, label %for.cond.cleanup.loopexit, label %for.body4.lr.ph

for.body11:                                       ; preds = %for.body11.preheader, %for.body11
  %indvars.iv54 = phi i64 [ %indvars.iv.next55, %for.body11 ], [ 0, %for.body11.preheader ]
  %arrayidx15 = getelementptr inbounds [10 x i32], ptr %a, i64 %indvars.iv54, i64 %indvars.iv58
  %2 = load i32, ptr %arrayidx15, align 4
  %3 = trunc i64 %indvars.iv54 to i32
  %add16 = add nsw i32 %2, %3
  %arrayidx20 = getelementptr inbounds [10 x i32], ptr %b, i64 %indvars.iv54, i64 %indvars.iv58
  store i32 %add16, ptr %arrayidx20, align 4
  %indvars.iv.next55 = add nuw nsw i64 %indvars.iv54, 1
  %exitcond57 = icmp eq i64 %indvars.iv.next55, %wide.trip.count56
  br i1 %exitcond57, label %for.cond.cleanup10, label %for.body11, !llvm.loop !0
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.fusion.disable"}
