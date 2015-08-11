; RUN: opt < %s -loop-simplify -hir-de-ssa | opt -analyze -hir-parser -hir-details | FileCheck %s

; Check parsing output for the temp blobs
; CHECK: DO i64 i1 = 0, %n + -1
; CHECK: <BLOB> LINEAR i32 %n
; CHECK: <BLOB> LINEAR i32* %B
; CHECK: <BLOB> LINEAR i32* %C
; CHECK: <BLOB> LINEAR i32* %A
; CHECK: <BLOB> NON-LINEAR i32 %0
; CHECK: <BLOB> NON-LINEAR i32 %1

; ModuleID = 'non-linear-blobs.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %A, i32* nocapture readonly %B, i32* nocapture readonly %C, i32 %n) {
entry:
  %cmp.10 = icmp sgt i32 %n, 0
  br i1 %cmp.10, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %C, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4
  %add = add nsw i32 %1, %0
  %arrayidx4 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %add, i32* %arrayidx4, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

