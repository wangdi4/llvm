; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Verify that the regions for the two inner loopnests are formed in lexical order.

; CHECK: BEGIN REGION
; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   %0 = (%B)[i1];
; CHECK: |   %1 = (%A)[i1];
; CHECK: |   (%A)[i1] = %0 + %1;
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: BEGIN REGION
; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   %2 = (%B)[i1];
; CHECK: |   %3 = (%C)[i1];
; CHECK: |   (%C)[i1] = %2 + %3;
; CHECK: + END LOOP
; CHECK: END REGION


source_filename = "lexical-order.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture %A, ptr nocapture readonly %B, ptr nocapture %C, i32 %n) {
entry:
  %cmp240 = icmp sgt i32 %n, 0
  %cmp744 = icmp sgt i32 %n, 1
  %wide.trip.count = sext i32 %n to i64
  %wide.trip.count49 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.inc22, %entry
  %i.046 = phi i32 [ 0, %entry ], [ %inc23, %for.inc22 ]
  br i1 %cmp240, label %for.body3.lr.ph, label %for.end

for.body3.lr.ph:                                  ; preds = %for.body
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %arrayidx5 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx5, align 4
  %add = add nsw i32 %1, %0
  store i32 %add, ptr %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body3

for.end.loopexit:                                 ; preds = %for.body3
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.body
  br i1 %cmp744, label %for.body8.lr.ph, label %for.inc22

for.body8.lr.ph:                                  ; preds = %for.end
  br label %for.body8

for.body8:                                        ; preds = %for.body8.lr.ph, %for.inc20
  %k.045 = phi i32 [ 1, %for.body8.lr.ph ], [ %mul, %for.inc20 ]
  br i1 %cmp240, label %for.body11.lr.ph, label %for.inc20

for.body11.lr.ph:                                 ; preds = %for.body8
  br label %for.body11

for.body11:                                       ; preds = %for.body11, %for.body11.lr.ph
  %indvars.iv47 = phi i64 [ 0, %for.body11.lr.ph ], [ %indvars.iv.next48, %for.body11 ]
  %arrayidx13 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv47
  %2 = load i32, ptr %arrayidx13, align 4
  %arrayidx15 = getelementptr inbounds i32, ptr %C, i64 %indvars.iv47
  %3 = load i32, ptr %arrayidx15, align 4
  %add16 = add nsw i32 %3, %2
  store i32 %add16, ptr %arrayidx15, align 4
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %exitcond50 = icmp eq i64 %indvars.iv.next48, %wide.trip.count49
  br i1 %exitcond50, label %for.inc20.loopexit, label %for.body11

for.inc20.loopexit:                               ; preds = %for.body11
  br label %for.inc20

for.inc20:                                        ; preds = %for.inc20.loopexit, %for.body8
  %mul = shl nsw i32 %k.045, 1
  %cmp7 = icmp slt i32 %mul, %n
  br i1 %cmp7, label %for.body8, label %for.inc22.loopexit

for.inc22.loopexit:                               ; preds = %for.inc20
  br label %for.inc22

for.inc22:                                        ; preds = %for.inc22.loopexit, %for.end
  %inc23 = add nuw nsw i32 %i.046, 1
  %exitcond51 = icmp eq i32 %inc23, 20
  br i1 %exitcond51, label %for.end24, label %for.body

for.end24:                                        ; preds = %for.inc22
  ret void
}

