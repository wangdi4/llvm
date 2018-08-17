; RUN: opt -loop-rotate -S < %s | FileCheck %s

; Verify that we rotate the loop so that the loop latch has IV comparison check.
; This makes the loop countable for loopopt.

; Check that while.body becomes the loop header after rotation.

; CHECK: entry:
; CHECK:  br i1 {{.*}}, label %while.end, label %while.body.lr.ph

; CHECK: while.body.lr.ph:
; CHECK-NEXT:  br label %while.body


target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"

define dso_local i32 @foo(i8* nocapture readonly %A, i8* nocapture readonly %B, i32 %s, i32 %n) "pre_loopopt" {
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %i.0 = phi i32 [ %s, %entry ], [ %inc, %while.body ]
  %inc = add i32 %i.0, 1
  %cmp = icmp eq i32 %i.0, %n
  br i1 %cmp, label %while.end, label %while.body

while.body:                                       ; preds = %while.cond
  %idxprom = zext i32 %inc to i64
  %arrayidx = getelementptr inbounds i8, i8* %A, i64 %idxprom
  %0 = load i8, i8* %arrayidx, align 1
  %arrayidx2 = getelementptr inbounds i8, i8* %B, i64 %idxprom
  %1 = load i8, i8* %arrayidx2, align 1
  %cmp4 = icmp eq i8 %0, %1
  br i1 %cmp4, label %while.cond, label %while.end

while.end:                                        ; preds = %while.body, %while.cond
  %inc.lcssa = phi i32 [ %inc, %while.body ], [ %inc, %while.cond ]
  ret i32 %inc.lcssa
}
