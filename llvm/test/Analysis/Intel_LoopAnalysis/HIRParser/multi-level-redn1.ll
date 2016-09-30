; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the intermediate liveout temp of multi-level reduction is handled properly.

; CHECK: LiveOuts: %t.026.out

; CHECK: + DO i1 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: |   %0 = (%A)[i1];
; CHECK: |   %t.026 = %0  +  %t.026;
; CHECK: |   %t.026.out = %t.026;
; CHECK: |   + DO i2 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: |   |   %1 = (%B)[i2];
; CHECK: |   |   %t.026 = %1  +  %t.026;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


source_filename = "multi-level-redn1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32* nocapture readonly %A, i32* nocapture readonly %B, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp24 = icmp sgt i32 %n, 0
  br i1 %cmp24, label %for.body3.preheader.preheader, label %for.end9

for.body3.preheader.preheader:                    ; preds = %entry
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.body3.preheader.preheader, %for.inc7
  %indvars.iv28 = phi i64 [ %indvars.iv.next29, %for.inc7 ], [ 0, %for.body3.preheader.preheader ]
  %t.026 = phi i32 [ %add6.lcssa, %for.inc7 ], [ 0, %for.body3.preheader.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv28
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %0, %t.026
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.preheader
  %indvars.iv = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next, %for.body3 ]
  %t.123 = phi i32 [ %add, %for.body3.preheader ], [ %add6, %for.body3 ]
  %arrayidx5 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx5, align 4
  %add6 = add nsw i32 %1, %t.123
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.body3
  %add6.lcssa = phi i32 [ %add6, %for.body3 ]
  %indvars.iv.next29 = add nuw nsw i64 %indvars.iv28, 1
  %lftr.wideiv30 = trunc i64 %indvars.iv.next29 to i32
  %exitcond31 = icmp eq i32 %lftr.wideiv30, %n
  br i1 %exitcond31, label %for.end9.loopexit, label %for.body3.preheader

for.end9.loopexit:                                ; preds = %for.inc7
  %add.lcssa = phi i32 [ %add, %for.inc7 ]
  br label %for.end9

for.end9:                                         ; preds = %for.end9.loopexit, %entry
  %s.0.lcssa = phi i32 [ 0, %entry ], [ %add.lcssa, %for.end9.loopexit ]
  ret i32 %s.0.lcssa
}

