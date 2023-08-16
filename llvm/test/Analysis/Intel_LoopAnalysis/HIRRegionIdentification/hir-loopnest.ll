; RUN: opt < %s -passes='print<hir-region-identification>' -disable-output 2>&1 | FileCheck %s

; Check output of hir-regions
; CHECK: Region 1
; CHECK-NEXT: EntryBB
; CHECK-SAME: for.cond1.preheader
; CHECK-NEXT: ExitBB
; CHECK-NEXT: Member
; CHECK-SAME: for.cond1.preheader
; CHECK-SAME: for.body3.preheader
; CHECK-SAME: for.body3
; CHECK-SAME: if.else
; CHECK-SAME: if.then
; CHECK-SAME: for.inc
; CHECK-SAME: for.inc14.loopexit
; CHECK-SAME: for.inc14


; Verify that region is throttled if the instruction threshold is too low.

; RUN: opt < %s -passes='print<hir-region-identification>' -hir-region-inst-threshold=1 -disable-output | FileCheck %s --check-prefix=INST-THRESHOLD

; INST-THRESHOLD-NOT: Region 1


; ModuleID = 'loopnest.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [100 x [100 x i32]] zeroinitializer, align 16
@B = common global [100 x i32] zeroinitializer, align 16

define void @foo(i32 %m, i32 %n) {
entry:
  %cmp25 = icmp sgt i32 %n, 0
  br i1 %cmp25, label %for.cond1.preheader.lr.ph, label %for.end16

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp223 = icmp sgt i32 %m, 0
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc14, %for.cond1.preheader.lr.ph
  %indvars.iv27 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next28, %for.inc14 ]
  br i1 %cmp223, label %for.body3.preheader, label %for.inc14

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body3.preheader ]
  %arrayidx5 = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 %indvars.iv27, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx5, align 4
  %cmp6 = icmp sgt i32 %0, 5
  br i1 %cmp6, label %if.then, label %if.else

if.then:                                          ; preds = %for.body3
  store i32 0, ptr %arrayidx5, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body3
  %arrayidx12 = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx12, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr %arrayidx12, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.else, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %m
  br i1 %exitcond, label %for.inc14.loopexit, label %for.body3

for.inc14.loopexit:                               ; preds = %for.inc
  br label %for.inc14

for.inc14:                                        ; preds = %for.inc14.loopexit, %for.cond1.preheader
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %lftr.wideiv29 = trunc i64 %indvars.iv.next28 to i32
  %exitcond30 = icmp eq i32 %lftr.wideiv29, %n
  br i1 %exitcond30, label %for.end16.loopexit, label %for.cond1.preheader

for.end16.loopexit:                               ; preds = %for.inc14
  br label %for.end16

for.end16:                                        ; preds = %for.end16.loopexit, %entry
  ret void
}
