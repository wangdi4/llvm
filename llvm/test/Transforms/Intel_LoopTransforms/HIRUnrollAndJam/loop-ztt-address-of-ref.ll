; RUN: opt < %s -hir-ssa-deconstruction -hir-unroll-and-jam -print-after=hir-unroll-and-jam -hir-details 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-unroll-and-jam,print<hir>" -aa-pipeline="basic-aa" -hir-details < %s 2>&1 | FileCheck %s

; Verify that unroll & jam does not assume that loop ddrefs only have a single
; canon expr. Ztt of loops with pointer IVs can have multiple dims.

; HIR-
; + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; |   + DO i2 = 0, (4 * sext.i32.i64(%n) + -4 * sext.i32.i64(%s) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; |   |   + DO i3 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; |   |   |   (@A)[0][i1 + i2 + i3 + sext.i32.i64(%s)] = i3;
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP


; CHECK: DO i32 i1
; CHECK: Ztt: if (&((@A)[0][%s]) <u &((@A)[0][%n]))
; CHECK: DO i64 i2

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local global [10 x i32] zeroinitializer, align 16

define dso_local void @foo(i32 %n, i32 %s) local_unnamed_addr {
entry:
  %cmp33 = icmp slt i32 0, %n
  br i1 %cmp33, label %for.body.lr.ph, label %for.end15

for.body.lr.ph:                                   ; preds = %entry
  %idxprom = sext i32 %s to i64
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @A, i64 0, i64 %idxprom
  %idxprom2 = sext i32 %n to i64
  %arrayidx3 = getelementptr inbounds [10 x i32], [10 x i32]* @A, i64 0, i64 %idxprom2
  %cmp430 = icmp ult i32* %arrayidx, %arrayidx3
  %cmp728 = icmp slt i32 0, %n
  br label %for.body

for.body:                                         ; preds = %for.cond.cleanup, %for.body.lr.ph
  %j.034 = phi i32 [ 0, %for.body.lr.ph ], [ %inc14, %for.cond.cleanup ]
  br i1 %cmp430, label %for.cond6.preheader.preheader, label %for.cond.cleanup

for.cond6.preheader.preheader:                    ; preds = %for.body
  br label %for.cond6.preheader

for.cond6.preheader:                              ; preds = %for.cond6.preheader.preheader, %for.inc11
  %p.031 = phi i32* [ %incdec.ptr, %for.inc11 ], [ %arrayidx, %for.cond6.preheader.preheader ]
  br i1 %cmp728, label %for.body8.preheader, label %for.inc11

for.body8.preheader:                              ; preds = %for.cond6.preheader
  br label %for.body8

for.cond.cleanup.loopexit:                        ; preds = %for.inc11
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %for.body
  %j.035 = phi i32 [ %j.034, %for.body ], [ %j.034, %for.cond.cleanup.loopexit ]
  %inc14 = add nuw nsw i32 %j.035, 1
  %cmp = icmp slt i32 %inc14, %n
  br i1 %cmp, label %for.body, label %for.end15.loopexit

for.body8:                                        ; preds = %for.body8.preheader, %for.body8
  %i.029 = phi i32 [ %inc, %for.body8 ], [ 0, %for.body8.preheader ]
  %add = add nuw nsw i32 %i.029, %j.034
  %idxprom9 = zext i32 %add to i64
  %arrayidx10 = getelementptr inbounds i32, i32* %p.031, i64 %idxprom9
  store i32 %i.029, i32* %arrayidx10, align 4
  %inc = add nuw nsw i32 %i.029, 1
  %cmp7 = icmp slt i32 %inc, %n
  br i1 %cmp7, label %for.body8, label %for.inc11.loopexit

for.inc11.loopexit:                               ; preds = %for.body8
  br label %for.inc11

for.inc11:                                        ; preds = %for.inc11.loopexit, %for.cond6.preheader
  %p.032 = phi i32* [ %p.031, %for.cond6.preheader ], [ %p.031, %for.inc11.loopexit ]
  %incdec.ptr = getelementptr inbounds i32, i32* %p.032, i64 1
  %cmp4 = icmp ult i32* %incdec.ptr, %arrayidx3
  br i1 %cmp4, label %for.cond6.preheader, label %for.cond.cleanup.loopexit

for.end15.loopexit:                               ; preds = %for.cond.cleanup
  br label %for.end15

for.end15:                                        ; preds = %for.end15.loopexit, %entry
  ret void
}
