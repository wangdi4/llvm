; RUN: opt < %s -hir-create-function-level-region -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -hir-details 2>&1 | FileCheck %s

; Verify that both sibling loops get their own ZTTs by duplicating the single guarding condition in the incoming IR.
; This is done to undo 'jump threading' and make analysis easier inside loopopt although it can cause some degradation if the duplicate condition is not cleaned up afterwards.


; CHECK:    %tB = (%B)[0];

; CHECK: + Ztt: if (%n > 0)
; CHECK: + DO i64 i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   (%A)[i1] = %tB;
; CHECK: + END LOOP


; CHECK:    %tA = (%A)[0];

; CHECK: + Ztt: if (%n > 0)
; CHECK: + Loop metadata: No
; CHECK: + DO i64 i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   (%B)[i1] = %tA;
; CHECK: + END LOOP

; CHECK: ret ;


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr nocapture %A, ptr nocapture %B, i32 %n) {
entry:
  %cmp20 = icmp sgt i32 %n, 0
  br i1 %cmp20, label %for.body.preheader, label %for.end8

for.body.preheader:                               ; preds = %entry
  %tB = load i32, ptr %B, align 4
  %wide.trip.count2426 = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv22 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next23, %for.body ]
  %ptridx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv22
  store i32 %tB, ptr %ptridx, align 4
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond25 = icmp eq i64 %indvars.iv.next23, %wide.trip.count2426
  br i1 %exitcond25, label %for.body3.preheader, label %for.body

for.body3.preheader:                              ; preds = %for.body3.preheader
  %tA = load i32, ptr %A, align 4
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next, %for.body3 ]
  %ptridx5 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  store i32 %tA, ptr %ptridx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count2426
  br i1 %exitcond, label %for.end8.loopexit, label %for.body3

for.end8.loopexit:                                ; preds = %for.body3
  br label %for.end8

for.end8:                                         ; preds = %for.end8.loopexit, %entry
  ret void
}

