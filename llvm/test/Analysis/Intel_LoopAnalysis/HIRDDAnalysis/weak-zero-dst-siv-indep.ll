; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 < %s  | FileCheck %s

; Test checks that Weak-Zero (dst) SIV test is able to calculate independence of
; (@A)[0][0] and (@A)[0][i1 + 1].

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %t0 = (@A)[0][i1 + 1];
; CHECK: |   %t1 = (@B)[0][i1];
; CHECK: |   (@A)[0][0] = %t0 + %t1;
; CHECK: + END LOOP

; CHECK: DD graph for function foo:
; CHECK: Region 0:
; CHECK-NOT: @A[0][i1 + 1]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx0 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv.next
  %t0 = load i32, ptr %arrayidx0, align 4
  %arrayidx = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv
  %t1 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %t1, %t0
  %arrayidx2 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 0 
  store i32 %add, ptr %arrayidx2, align 4
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

