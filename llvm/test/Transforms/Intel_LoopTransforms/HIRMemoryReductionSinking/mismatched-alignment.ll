; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-memory-reduction-sinking -print-after=hir-memory-reduction-sinking < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking,print<hir-framework>" 2>&1 < %s | FileCheck %s

; Verify that we give up if the dependence is between reductions having
; different alignment (4 and 1) as handling partial overlap is tricky.

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, 49, 1   <DO_LOOP>
; CHECK: |   %0 = (%A)[5];
; CHECK: |   (%A)[5] = %0 + 2;
; CHECK: |   %1 = (%B)[i1];
; CHECK: |   (%B)[i1] = %1 + 3;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %A, i32* nocapture %B) {
entry:
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 5
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %0, 2
  store i32 %add, i32* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx1, align 1
  %add2 = add nsw i32 %1, 3
  store i32 %add2, i32* %arrayidx1, align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 50
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

