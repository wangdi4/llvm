; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the && condition is successfully propagated to the if condition.

; CHECK: + DO i1 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: |   %0 = (%A)[i1];
; CHECK: |   %2 = (%A)[i1 + 1];
; CHECK: |   if (%0 > 5 && %2 < 10)
; CHECK: |   {
; CHECK: |      (%B)[i1] = %0;
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      (%B)[i1] = -1 * %2;
; CHECK: |   }
; CHECK: + END LOOP


; ModuleID = 'and1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture readonly %A, i32* nocapture %B, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp26 = icmp sgt i32 %n, 0
  br i1 %cmp26, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ %1, %for.inc ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %0, 5
  %1 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx3 = getelementptr inbounds i32, i32* %A, i64 %1
  %2 = load i32, i32* %arrayidx3, align 4
  %cmp4 = icmp slt i32 %2, 10
  %or.cond = and i1 %cmp1, %cmp4
  br i1 %or.cond, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx8 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  store i32 %0, i32* %arrayidx8, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  %sub = sub nsw i32 0, %2
  %arrayidx13 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  store i32 %sub, i32* %arrayidx13, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %lftr.wideiv = trunc i64 %1 to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

