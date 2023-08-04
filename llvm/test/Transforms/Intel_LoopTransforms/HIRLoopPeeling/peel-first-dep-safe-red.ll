; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-peeling" -disable-output -debug-only=hir-loop-peeling 2>&1 < %s  | FileCheck %s

; Verify that we can ignore safe reductions in loop peeling candidates.

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %0 = (@A)[0][0];
; CHECK: |   %1 = (@B)[0][i1];
; CHECK: |   (@A)[0][i1] = %0 + %1;
; CHECK: |   %t = %t  +  %1;
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: Peelable dependency found!
; CHECK: Loop peeling candidate found!

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %t = phi i32 [ 0, %entry ], [ %t.add, %for.body ]
  %0 = load i32, ptr getelementptr inbounds ([100 x i32], ptr @A, i64 0, i64 0), align 16
  %arrayidx = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %1, %0
  %arrayidx2 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv
  store i32 %add, ptr %arrayidx2, align 4
  %t.add = add nsw i32 %t, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %t.lcssa = phi i32 [ %t.add, %for.body ]
  ret void
}

