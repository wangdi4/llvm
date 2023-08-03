; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lmm,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we do not hoist load of (%A)[1] outside the loop. DD was using invalid upper bound of zero for unknown loops to construct edges.

; Incoming HIR-
; + UNKNOWN LOOP i1
; |   <i1 = 0>
; |   while.body:
; |   %2 = (%A)[1];
; |   (%A)[i1] = %2 + %1;
; |   %3 = (%A)[i1 + 1];
; |   %1 = %3;
; |   if (%3 != 0)
; |   {
; |      <i1 = i1 + 1>
; |      goto while.body;
; |   }
; + END LOOP

; CHECK-NOT: modified

; CHECK: UNKNOWN LOOP i1


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(ptr nocapture %A) local_unnamed_addr #0 {
entry:
  %0 = load i32, ptr %A, align 4
  %cmp9 = icmp eq i32 %0, 0
  br i1 %cmp9, label %while.end, label %while.body.lr.ph

while.body.lr.ph:                                 ; preds = %entry
  %arrayidx1 = getelementptr inbounds i32, ptr %A, i64 1
  br label %while.body

while.body:                                       ; preds = %while.body.lr.ph, %while.body
  %indvars.iv = phi i64 [ 0, %while.body.lr.ph ], [ %indvars.iv.next, %while.body ]
  %1 = phi i32 [ %0, %while.body.lr.ph ], [ %3, %while.body ]
  %arrayidx11 = phi ptr [ %A, %while.body.lr.ph ], [ %arrayidx, %while.body ]
  %2 = load i32, ptr %arrayidx1, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, ptr %arrayidx11, align 4
  %indvars.iv.next = add nuw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv.next
  %3 = load i32, ptr %arrayidx, align 4
  %cmp = icmp eq i32 %3, 0
  br i1 %cmp, label %while.end.loopexit, label %while.body

while.end.loopexit:                               ; preds = %while.body
  br label %while.end

while.end:                                        ; preds = %while.end.loopexit, %entry
  ret void
}

