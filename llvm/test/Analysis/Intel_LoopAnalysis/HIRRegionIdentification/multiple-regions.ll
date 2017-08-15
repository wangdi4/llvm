; RUN: opt < %s -analyze -hir-region-identification | FileCheck %s

; Check output of hir-regions
; CHECK: Region 1
; CHECK-NEXT: EntryBB: for.body
; CHECK-NEXT: ExitBB
; CHECK-NEXT: Member
; CHECK-SAME: for.body
; CHECK: Region 2
; CHECK-NEXT: EntryBB: for.body4
; CHECK-NEXT: ExitBB
; CHECK-NEXT: Member
; CHECK-SAME: for.body4


; Check that a single region is formed with -hir-create-function-level-region=true.

; RUN: opt < %s -analyze -hir-region-identification -hir-create-function-level-region=true | FileCheck %s -check-prefix=FUNC-REG

; FUNC-REG: Region 1
; FUNC-REG-NEXT: EntryBB: entry

; FUNC-REG-NOT: Region 2


; ModuleID = 'loops.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [100 x i32] zeroinitializer, align 16
@A = common global [100 x i32] zeroinitializer, align 16

define void @foo(i32 %m, i32 %n) {
entry:
  %cmp16 = icmp sgt i32 %n, 0
  br i1 %cmp16, label %for.body.preheader, label %for.cond2.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond2.preheader.loopexit:                     ; preds = %for.body
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.cond2.preheader.loopexit, %entry
  %cmp314 = icmp sgt i32 %m, 0
  br i1 %cmp314, label %for.body4.preheader, label %for.end9

for.body4.preheader:                              ; preds = %for.cond2.preheader
  br label %for.body4

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv18 = phi i64 [ %indvars.iv.next19, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv18
  %0 = load i32, i32* %arrayidx, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %arrayidx, align 4
  %indvars.iv.next19 = add nuw nsw i64 %indvars.iv18, 1
  %lftr.wideiv20 = trunc i64 %indvars.iv.next19 to i32
  %exitcond21 = icmp eq i32 %lftr.wideiv20, %n
  br i1 %exitcond21, label %for.cond2.preheader.loopexit, label %for.body

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body4 ], [ 0, %for.body4.preheader ]
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  store i32 0, i32* %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %m
  br i1 %exitcond, label %for.end9.loopexit, label %for.body4

for.end9.loopexit:                                ; preds = %for.body4
  br label %for.end9

for.end9:                                         ; preds = %for.end9.loopexit, %for.cond2.preheader
  ret void
}
