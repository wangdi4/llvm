; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-scalarrepl-array" -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -hir-scalarrepl-array-depdist-threshold=1 -disable-output < %s 2>&1 | FileCheck %s

; Verify that we are able to scalar-replace adjacent groups {(%A)[i1], (%A)[i1 + 1]}
; and {(%A)[i1 + 2], (%A)[i1 + 3]} when distance threshold is set to 1.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK: |   %ld = (%A)[i1];
; CHECK: |   (%A)[i1 + 1] = %ld + 5;
; CHECK: |   %ld1 = (%A)[i1 + 2];
; CHECK: |   (%A)[i1 + 3] = %ld1 + 10;
; CHECK: + END LOOP
; CHECK: END REGION


; CHECK: Dump After

; CHECK: %scalarepl = (%A)[0];
; CHECK: %scalarepl4 = (%A)[2];
; CHECK: + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK: |   %ld = %scalarepl;
; CHECK: |   %scalarepl1 = %ld + 5;
; CHECK: |   (%A)[i1 + 1] = %scalarepl1;
; CHECK: |   %ld1 = %scalarepl4;
; CHECK: |   %scalarepl5 = %ld1 + 10;
; CHECK: |   (%A)[i1 + 3] = %scalarepl5;
; CHECK: |   %scalarepl = %scalarepl1;
; CHECK: |   %scalarepl4 = %scalarepl5;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr nocapture noundef %A) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %ld = load i32, ptr %arrayidx, align 4
  %ld.add = add i32 %ld, 5
  %iv.add1 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx1 = getelementptr inbounds i32, ptr %A, i64 %iv.add1
  store i32 %ld.add, ptr %arrayidx1, align 4
  %iv.add2 = add nuw nsw i64 %indvars.iv, 2
  %arrayidx2 = getelementptr inbounds i32, ptr %A, i64 %iv.add2
  %ld1 = load i32, ptr %arrayidx2, align 4
  %ld.add1 = add i32 %ld1, 10
  %iv.add3 = add nuw nsw i64 %indvars.iv, 3
  %arrayidx3 = getelementptr inbounds i32, ptr %A, i64 %iv.add3
  store i32 %ld.add1, ptr %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

