; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that we do not eliminate the store to (%A)[0] by propagating its value to the
; load because the store inside i2 loop reaches the load.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 49, 1   <DO_LOOP>
; CHECK: |   (%A)[0] = i1;
; CHECK: |
; CHECK: |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   %ld = (%A)[0];
; CHECK: |   |   (%A)[0] = 5;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK-NOT: modified


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo() {
entry:
  %A = alloca i32, align 16
  br label %outer.preheader

outer.preheader:                               ; preds = %entry
  br label %outer

outer:                                         ; preds = %outer.preheader, %outer
  %indvars.iv21 = phi i64 [ 0, %outer.preheader ], [ %indvars.iv.next22, %latch ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 0
  %0 = trunc i64 %indvars.iv21 to i32
  store i32 %0, i32* %arrayidx, align 4
  br label %inner

inner:                                        ; preds = %outer, %inner
  %indvars.iv = phi i64 [ 0, %outer ], [ %indvars.iv.next, %inner ]
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 0
  %ld = load i32, i32* %arrayidx5, align 4
  store i32 5, i32* %arrayidx5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %latch, label %inner

latch:
  %ld.lcssa = phi i32 [ %ld, %inner]
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, 1
  %cmp = icmp eq i64 %indvars.iv.next22, 50
  br i1 %cmp, label %for.end8.loopexit, label %outer

for.end8.loopexit:                                ; preds = %inner
  %ld.lcssa1 = phi i32 [ %ld.lcssa, %latch ]
  br label %for.end8

for.end8:                                         ; preds = %for.end8.loopexit
  %ld.out = phi i32 [ %ld.lcssa1, %for.end8.loopexit ]
  ret i32 %ld.out
}

