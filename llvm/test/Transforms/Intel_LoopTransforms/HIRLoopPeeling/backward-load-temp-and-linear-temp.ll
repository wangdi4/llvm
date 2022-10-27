; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-peeling" -disable-output -print-before=hir-loop-peeling -print-after=hir-loop-peeling 2>&1 < %s  | FileCheck %s

; Verify that we recognize %t as backwards substitutible load, %s as backwards
; substitutible linear temp and peel the loop.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   (@A)[0][i1] = %t + %s;
; CHECK: |   %t = (@B)[0][i1];
; CHECK: |   %s = i1 + 1;
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: (@A)[0][0] = %t + %s;

; CHECK: + DO i1 = 0, 98, 1   <DO_LOOP>
; CHECK: |   %t = (@B)[0][i1];
; CHECK: |   (@A)[0][i1 + 1] = i1 + %t + 1;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %t = phi i32 [ 5, %entry ], [ %0, %for.body ]
  %s = phi i32 [ 5, %entry ], [ %s.inc, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %add = add nsw i32 %t, %s
  store i32 %add, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %s.inc = trunc i64 %indvars.iv.next to i32
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
