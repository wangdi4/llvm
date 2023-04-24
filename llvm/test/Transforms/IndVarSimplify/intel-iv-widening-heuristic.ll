; In the following IR widening %j.020 requires a widened decrementing IV in the outer loop because its initial value is (45 - i).
; Verify that the inner loop IV is not widened when the threshold for new parent loop IVs allowed is set to zero or deep loopnest threshold is set <=2.

; RUN: opt -passes="loop(indvars)" -indvars-new-parent-loop-iv-threshold=0 -indvars-deep-loopnest-threshold=2 -S < %s | FileCheck %s --check-prefix=NO-WIDENING
; RUN: opt -passes="loop(indvars)" -indvars-new-parent-loop-iv-threshold=1 -indvars-deep-loopnest-threshold=2 -S < %s | FileCheck %s --check-prefix=WIDENING
; RUN: opt -passes="loop(indvars)" -indvars-new-parent-loop-iv-threshold=0 -indvars-deep-loopnest-threshold=3 -S < %s | FileCheck %s --check-prefix=WIDENING

; NO-WIDENING: for.body:
; NO-WIDENING: %indvars.iv{{.*}} = phi i64

; NO-WIDENING: for.body4:
; NO-WIDENING: %j.020 = phi i32

; WIDENING: for.body:
; WIDENING: %indvars.iv{{.*}} = phi i64
; WIDENING: %indvars.iv{{.*}} = phi i64

; WIDENING: for.body4:
; WIDENING: %indvars.iv{{.*}} = phi i64

; WIDENING: for.inc7:
; WIDENING: %indvars.iv.next{{.*}} = add nsw i64 %indvars.iv{{.*}}, -1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %A, i32 %n, i32* nocapture %B) local_unnamed_addr #0 {
entry:
  %cmp21 = icmp sgt i32 %n, 0
  br i1 %cmp21, label %for.body.preheader, label %for.end9

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc7
  %i.022 = phi i32 [ %inc8, %for.inc7 ], [ 0, %for.body.preheader ]
  %idxprom = zext i32 %i.022 to i64
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %idxprom
  store i32 %i.022, i32* %arrayidx, align 4
  %sub = sub nsw i32 45, %i.022
  br label %for.body4

for.body4:                                        ; preds = %for.body4, %for.body
  %j.020 = phi i32 [ %sub, %for.body ], [ %inc, %for.body4 ]
  %idxprom5 = sext i32 %j.020 to i64
  %arrayidx6 = getelementptr inbounds i32, i32* %A, i64 %idxprom5
  store i32 %j.020, i32* %arrayidx6, align 4
  %inc = add nsw i32 %j.020, 1
  %cmp3 = icmp sle i32 %j.020, %sub
  br i1 %cmp3, label %for.body4, label %for.inc7

for.inc7:                                         ; preds = %for.body4
  %inc8 = add nuw nsw i32 %i.022, 1
  %cmp = icmp slt i32 %inc8, %n
  br i1 %cmp, label %for.body, label %for.end9.loopexit

for.end9.loopexit:                                ; preds = %for.inc7
  br label %for.end9

for.end9:                                         ; preds = %for.end9.loopexit, %entry
  ret void
}

