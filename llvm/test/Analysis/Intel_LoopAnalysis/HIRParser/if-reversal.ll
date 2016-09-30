; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the if compare is inverted so as to move then children into the empty then case.

; CHECK: + DO i1 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: |   %0 = (%A)[i1];
; CHECK: |   if (%0 != 0)
; CHECK: |   {
; CHECK: |      (%A)[i1] = %0 + 1;
; CHECK: |      %1 = (%B)[i1];
; CHECK: |      (%B)[i1] = %0 + %1 + 1;
; CHECK: |   }
; CHECK: + END LOOP


source_filename = "if-reversal.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %A, i32* nocapture %B, i32 %n) #0 {
entry:
  %cmp17 = icmp sgt i32 %n, 0
  br i1 %cmp17, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp eq i32 %0, 0
  br i1 %cmp1, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %arrayidx, align 4
  %arrayidx7 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx7, align 4
  %add = add nsw i32 %1, %inc
  store i32 %add, i32* %arrayidx7, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

