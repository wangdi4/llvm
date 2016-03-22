; RUN: opt < %s -loop-simplify | opt -analyze -hir-region-identification | FileCheck %s

; Check output of hir-regions
; CHECK: Region 1
; CHECK-NEXT: EntryBB
; CHECK-SAME: for.body
; CHECK-NEXT: ExitBB
; CHECK-NEXT: Member
; CHECK-SAME: for.body
; CHECK: Region 2
; CHECK-NEXT: EntryBB
; CHECK-SAME: for.body4
; CHECK-NEXT: ExitBB
; CHECK-NEXT: Member
; CHECK-SAME: for.body4

; ModuleID = 'loops.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [100 x i32] zeroinitializer, align 16
@A = common global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %m, i32 %n) {
entry:
  %cmp16 = icmp sgt i32 %n, 0
  br i1 %cmp16, label %for.body, label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.body, %entry
  %cmp314 = icmp sgt i32 %m, 0
  br i1 %cmp314, label %for.body4, label %for.end9

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv18 = phi i64 [ %indvars.iv.next19, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv18
  %0 = load i32, i32* %arrayidx, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %arrayidx, align 4
  %indvars.iv.next19 = add nuw nsw i64 %indvars.iv18, 1
  %lftr.wideiv20 = trunc i64 %indvars.iv.next19 to i32
  %exitcond21 = icmp eq i32 %lftr.wideiv20, %n
  br i1 %exitcond21, label %for.cond2.preheader, label %for.body

for.body4:                                        ; preds = %for.cond2.preheader, %for.body4
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body4 ], [ 0, %for.cond2.preheader ]
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  store i32 0, i32* %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %m
  br i1 %exitcond, label %for.end9, label %for.body4

for.end9:                                         ; preds = %for.body4, %for.cond2.preheader
  ret void
}

