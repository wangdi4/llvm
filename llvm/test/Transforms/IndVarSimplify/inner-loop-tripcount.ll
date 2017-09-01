; RUN: opt < %s -indvars | opt -analyze -scalar-evolution | FileCheck %s

; Verfiy that we get a simplified backedge taken count for the inner loop in a
; triangular loopnest after indvars.

; Verify that indvars got triggered by checking for a 64 bit IV.
; CHECK: %indvars.iv{{[0-9]+}} = phi i64

; The IR should not contain any zext after indvars.
; CHECK-NOT: zext

; Backedge taken count for inner loop should still be in terms of the outer loop
; IV.
; CHECK: Loop %for.inner: backedge-taken count is {-2,+,1}<nw><%for.outer>

; CHECK: Loop %for.outer: backedge-taken count is 39


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

define void @foo(i32* nocapture %A) local_unnamed_addr #0 {
entry:
  br label %for.outer

for.outer:                              ; preds = %entry, %for.inc4
  %i.018 = phi i32 [ 0, %entry ], [ %inc5, %for.inc4 ]
  %sub = add nsw i32 %i.018, -1
  %cmp216 = icmp sgt i32 %i.018, 1
  br i1 %cmp216, label %for.inner.preheader, label %for.inc4

for.inner.preheader:                              ; preds = %for.outer
  br label %for.inner

for.inner:                                        ; preds = %for.inner.preheader, %for.inner
  %j.017 = phi i32 [ %inc, %for.inner ], [ 0, %for.inner.preheader ]
  %add = add nsw i32 %j.017, %i.018
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  store i32 %j.017, i32* %arrayidx, align 4
  %inc = add nsw i32 %j.017, 1
  %cmp2 = icmp slt i32 %inc, %sub
  br i1 %cmp2, label %for.inner, label %for.inc4.loopexit

for.inc4.loopexit:                                ; preds = %for.inner
  br label %for.inc4

for.inc4:                                         ; preds = %for.inc4.loopexit, %for.outer
  %inc5 = add nsw i32 %i.018, 1
  %cmp = icmp slt i32 %inc5, 40
  br i1 %cmp, label %for.outer, label %for.end6

for.end6:                                         ; preds = %for.inc4
  ret void
}

