; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Verify that %add.ptr24 which has an AddRec form with null base is handled successfully.
; Previously, we were compfailing on it.

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   loop:
; CHECK: |   %t4 = (%add.ptr24)[0].0;
; CHECK: |   if (%t4 != 0)
; CHECK: |   {
; CHECK: |      goto loop.cleanup_crit_edge;
; CHECK: |   }
; CHECK: |   if (i1 + 1 == %1)
; CHECK: |   {
; CHECK: |      goto for.cond.cleanup_crit_edge;
; CHECK: |   }
; CHECK: |   %add.ptr = &((null)[sext.i32.i64(%0) * i1 + sext.i32.i64(%0)]);
; CHECK: |   %3 = (%add.ptr)[0].1;
; CHECK: |   %add.ptr24 = &((null)[sext.i32.i64(%0) * i1 + sext.i32.i64(%0)]);
; CHECK: |   if (%3 == 0)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto loop;
; CHECK: |   }
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.j = type { i32, i32 }

define dso_local noundef zeroext i1 @foo() {
entry:
  %0 = load i32, ptr inttoptr (i64 4 to ptr), align 4
  %conv = sext i32 %0 to i64
  %1 = load i32, ptr inttoptr (i64 8 to ptr), align 8
  %cmp16 = icmp slt i32 %1, 1
  br i1 %cmp16, label %cleanup, label %latch.preheader

latch.preheader:                               ; preds = %entry
  %2 = zext i32 %1 to i64
  br label %loop.lr.ph

loop.lr.ph:                                     ; preds = %latch.preheader
  br label %loop

loop:                                           ; preds = %loop.lr.ph, %latch
  %add.ptr24 = phi ptr [ null, %loop.lr.ph ], [ %add.ptr, %latch ]
  %indvars.iv22 = phi i64 [ 0, %loop.lr.ph ], [ %indvars.iv.next, %latch ]
  %n = getelementptr inbounds %struct.j, ptr %add.ptr24, i64 0, i32 0
  %t4 = load i32, ptr %n, align 4
  %tobool2.not = icmp eq i32 %t4, 0
  br i1 %tobool2.not, label %for.cond, label %loop.cleanup_crit_edge

for.cond:                                         ; preds = %loop
  %indvars.iv.next = add nuw nsw i64 %indvars.iv22, 1
  %cmp = icmp uge i64 %indvars.iv.next, %2
  %exitcond = icmp eq i64 %indvars.iv.next, %2
  br i1 %exitcond, label %for.cond.cleanup_crit_edge, label %latch

latch:                                         ; preds = %for.cond
  %mul = mul nsw i64 %indvars.iv.next, %conv
  %add.ptr = getelementptr inbounds i8, ptr null, i64 %mul
  %o = getelementptr inbounds %struct.j, ptr %add.ptr, i64 0, i32 1
  %3 = load i32, ptr %o, align 4
  %tobool.not = icmp eq i32 %3, 0
  br i1 %tobool.not, label %loop, label %latch.cleanup_crit_edge

loop.cleanup_crit_edge:                ; preds = %loop
  br label %cleanup

latch.cleanup_crit_edge:              ; preds = %latch
  br label %cleanup

for.cond.cleanup_crit_edge:              ; preds = %for.cond
  br label %cleanup

cleanup:                                          ; preds = %cleanup, %entry
  ret i1 0
}

