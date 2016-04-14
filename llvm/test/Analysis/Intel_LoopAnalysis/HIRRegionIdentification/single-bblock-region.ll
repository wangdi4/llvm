; RUN: opt < %s -analyze -hir-region-identification | FileCheck %s

; Check output of hir-regions
; CHECK: Region 1
; CHECK-NEXT: EntryBB
; CHECK-SAME: for.body
; CHECK-NEXT: ExitBB
; CHECK-NEXT: Member
; CHECK-SAME: for.body

; ModuleID = 'q1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [10 x i32] zeroinitializer, align 16
@A = common global [10 x i32] zeroinitializer, align 16

define void @foo() {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.05 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %i.05
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds [10 x i32], [10 x i32]* @A, i64 0, i64 %i.05
  store i32 %0, i32* %arrayidx1, align 4
  %inc = add nuw nsw i64 %i.05, 1
  %exitcond = icmp eq i64 %inc, 6
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
