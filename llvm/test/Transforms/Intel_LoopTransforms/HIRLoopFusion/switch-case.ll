; Check that loop fusions works for loops in an HLSwitch construct.
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; The IR is generated from the following C code:
; void foo(int a, int outer, int inner, int *arr) {
;   for (int i = 0; i < outer; i++) {
;     switch (a) {
;       case 0: {
;         for (int j = 0; j < inner; j++)
;           arr[j] += 1;
;         for (int j = 0; j < inner; j++)
;           arr[j] += 2;
;         break;
;       }
;       case 1: {
;         for (int j = 0; j < inner; j++)
;           arr[j] += 3;
;         for (int j = 0; j < inner; j++)
;           arr[j] += 4;
;         break;
;       }
;       case 2: {
;         for (int j = 0; j < inner; j++)
;           arr[j] += 5;
;         for (int j = 0; j < inner; j++)
;           arr[j] += 6;
;         break;
;       }
;       default:
;         __builtin_unreachable();
;     }
;   }
; }

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, %outer + -1, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:       |   switch(%a)
; CHECK:       |   {
; CHECK:       |   case 0:
; CHECK:       |      + DO i2 = 0, zext.i32.i64(%inner) + -1, 1   <DO_LOOP>
; CHECK:       |      |   %0 = (%arr)[i2];
; CHECK:       |      |   (%arr)[i2] = %0 + 1;
; CHECK-NEXT:  |      |   %1 = (%arr)[i2];
; CHECK:       |      |   (%arr)[i2] = %1 + 2;
; CHECK:       |      + END LOOP
; CHECK:       |      break;
; CHECK:       |   case 1:
; CHECK:       |      + DO i2 = 0, zext.i32.i64(%inner) + -1, 1   <DO_LOOP>
; CHECK:       |      |   %2 = (%arr)[i2];
; CHECK:       |      |   (%arr)[i2] = %2 + 3;
; CHECK-NEXT:  |      |   %3 = (%arr)[i2];
; CHECK:       |      |   (%arr)[i2] = %3 + 4;
; CHECK:       |      + END LOOP
; CHECK:       |      break;
; CHECK:       |   case 2:
; CHECK:       |      + DO i2 = 0, zext.i32.i64(%inner) + -1, 1   <DO_LOOP>
; CHECK:       |      |   %4 = (%arr)[i2];
; CHECK:       |      |   (%arr)[i2] = %4 + 5;
; CHECK-NEXT:  |      |   %5 = (%arr)[i2];
; CHECK:       |      |   (%arr)[i2] = %5 + 6;
; CHECK:       |      + END LOOP
; CHECK:       |      break;
; CHECK:       |   default:
; CHECK:       |      goto sw.default;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32 noundef %a, i32 noundef %outer, i32 noundef %inner, ptr nocapture noundef %arr) {
entry:
  %cmp102 = icmp sgt i32 %outer, 0
  br i1 %cmp102, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:
  %cmp4290 = icmp sgt i32 %inner, 0
  %wide.trip.count = zext i32 %inner to i64
  br label %for.body

for.cond.cleanup.loopexit:
  br label %for.cond.cleanup

for.cond.cleanup:
  ret void

for.body:
  %i.0103 = phi i32 [ 0, %for.body.lr.ph ], [ %inc63, %for.inc62 ]
  switch i32 %a, label %sw.default [
    i32 0, label %for.cond1.preheader
    i32 1, label %for.cond18.preheader
    i32 2, label %for.cond41.preheader
  ]

for.cond41.preheader:
  br i1 %cmp4290, label %for.body44.preheader, label %for.inc62

for.body44.preheader:
  br label %for.body44

for.cond18.preheader:
  br i1 %cmp4290, label %for.body21.preheader, label %for.inc62

for.body21.preheader:
  br label %for.body21

for.cond1.preheader:
  br i1 %cmp4290, label %for.body4.preheader, label %for.inc62

for.body4.preheader:
  br label %for.body4

for.cond6.preheader:
  br label %for.body9

for.body4:
  %indvars.iv118 = phi i64 [ %indvars.iv.next119, %for.body4 ], [ 0, %for.body4.preheader ]
  %arrayidx = getelementptr inbounds i32, ptr %arr, i64 %indvars.iv118
  %0 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %0, 1
  store i32 %add, ptr %arrayidx, align 4
  %indvars.iv.next119 = add nuw nsw i64 %indvars.iv118, 1
  %exitcond121.not = icmp eq i64 %indvars.iv.next119, %wide.trip.count
  br i1 %exitcond121.not, label %for.cond6.preheader, label %for.body4

for.body9:
  %indvars.iv122 = phi i64 [ %indvars.iv.next123, %for.body9 ], [ 0, %for.cond6.preheader ]
  %arrayidx11 = getelementptr inbounds i32, ptr %arr, i64 %indvars.iv122
  %1 = load i32, ptr %arrayidx11, align 4
  %add12 = add nsw i32 %1, 2
  store i32 %add12, ptr %arrayidx11, align 4
  %indvars.iv.next123 = add nuw nsw i64 %indvars.iv122, 1
  %exitcond125.not = icmp eq i64 %indvars.iv.next123, %wide.trip.count
  br i1 %exitcond125.not, label %for.inc62.loopexit, label %for.body9

for.cond29.preheader:
  br label %for.body32

for.body21:
  %indvars.iv110 = phi i64 [ %indvars.iv.next111, %for.body21 ], [ 0, %for.body21.preheader ]
  %arrayidx23 = getelementptr inbounds i32, ptr %arr, i64 %indvars.iv110
  %2 = load i32, ptr %arrayidx23, align 4
  %add24 = add nsw i32 %2, 3
  store i32 %add24, ptr %arrayidx23, align 4
  %indvars.iv.next111 = add nuw nsw i64 %indvars.iv110, 1
  %exitcond113.not = icmp eq i64 %indvars.iv.next111, %wide.trip.count
  br i1 %exitcond113.not, label %for.cond29.preheader, label %for.body21

for.body32:
  %indvars.iv114 = phi i64 [ %indvars.iv.next115, %for.body32 ], [ 0, %for.cond29.preheader ]
  %arrayidx34 = getelementptr inbounds i32, ptr %arr, i64 %indvars.iv114
  %3 = load i32, ptr %arrayidx34, align 4
  %add35 = add nsw i32 %3, 4
  store i32 %add35, ptr %arrayidx34, align 4
  %indvars.iv.next115 = add nuw nsw i64 %indvars.iv114, 1
  %exitcond117.not = icmp eq i64 %indvars.iv.next115, %wide.trip.count
  br i1 %exitcond117.not, label %for.inc62.loopexit129, label %for.body32

for.cond52.preheader:
  br label %for.body55

for.body44:
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body44 ], [ 0, %for.body44.preheader ]
  %arrayidx46 = getelementptr inbounds i32, ptr %arr, i64 %indvars.iv
  %4 = load i32, ptr %arrayidx46, align 4
  %add47 = add nsw i32 %4, 5
  store i32 %add47, ptr %arrayidx46, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond52.preheader, label %for.body44

for.body55:
  %indvars.iv106 = phi i64 [ %indvars.iv.next107, %for.body55 ], [ 0, %for.cond52.preheader ]
  %arrayidx57 = getelementptr inbounds i32, ptr %arr, i64 %indvars.iv106
  %5 = load i32, ptr %arrayidx57, align 4
  %add58 = add nsw i32 %5, 6
  store i32 %add58, ptr %arrayidx57, align 4
  %indvars.iv.next107 = add nuw nsw i64 %indvars.iv106, 1
  %exitcond109.not = icmp eq i64 %indvars.iv.next107, %wide.trip.count
  br i1 %exitcond109.not, label %for.inc62.loopexit130, label %for.body55

sw.default:
  unreachable

for.inc62.loopexit:
  br label %for.inc62

for.inc62.loopexit129:
  br label %for.inc62

for.inc62.loopexit130:
  br label %for.inc62

for.inc62:
  %inc63 = add nuw nsw i32 %i.0103, 1
  %exitcond126.not = icmp eq i32 %inc63, %outer
  br i1 %exitcond126.not, label %for.cond.cleanup.loopexit, label %for.body
}
