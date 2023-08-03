; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,hir-lmm,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that Partial Predicate optimization will not trigger because of %0 -> %0 output edge.

; CHECK:     BEGIN REGION { modified }
; CHECK:              %[[LIMM:limm[0-9]*]] = (undef)[0];
; CHECK:              %[[LIMM2:limm[0-9]*]] = (%q)[0][1];
; CHECK:              %[[LIMM3:limm[0-9]*]] = (%q)[0][2];
; CHECK:              %[[LIMM4:limm[0-9]*]] = (%q)[0][3];
; CHECK:           + DO i1 = 0, 78, 1   <DO_LOOP>
; CHECK:           |   %0 = %[[LIMM]];
; CHECK:           |   if (%[[LIMM2]] <u %0)
; CHECK:           |   {
; CHECK:           |      (null)[0] = undef;
; CHECK:           |   }
; CHECK:           |   if (%[[LIMM3]] <u %0)
; CHECK:           |   {
; CHECK:           |      (null)[0] = undef;
; CHECK:           |   }
; CHECK:           |   if (%[[LIMM4]] <u %0)
; CHECK:           |   {
; CHECK:           |      (null)[0] = undef;
; CHECK:           |   }
; CHECK:           |   %0 = %[[LIMM]];
; CHECK:           |   if (%[[LIMM2]] <u %0)
; CHECK:           |   {
; CHECK:           |      (null)[0] = undef;
; CHECK:           |   }
; CHECK:           |   if (%[[LIMM3]] <u %0)
; CHECK:           |   {
; CHECK:           |      (null)[0] = undef;
; CHECK:           |   }
; CHECK:           |   if (%[[LIMM4]] <u %0)
; CHECK:           |   {
; CHECK:           |      (null)[0] = undef;
; CHECK:           |   }
; CHECK:           + END LOOP
; CHECK-NOT: DO i1
; CHECK:     END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @main() local_unnamed_addr #0 {
entry:
  %q = alloca [100 x i32], align 16
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %i.034 = phi i32 [ 2, %entry ], [ %inc20, %for.cond.cleanup3 ]
  br label %for.cond5.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond5.preheader:                              ; preds = %for.cond.cleanup7, %for.cond1.preheader
  %indvars.iv35 = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next36, %for.cond.cleanup7 ]
  %0 = load i32, ptr undef, align 4
  br label %for.body8

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %inc20 = add nuw nsw i32 %i.034, 1
  %exitcond38 = icmp eq i32 %inc20, 81
  br i1 %exitcond38, label %for.cond.cleanup, label %for.cond1.preheader

for.cond.cleanup7:                                ; preds = %for.inc
  %indvars.iv.next36 = add nuw nsw i64 %indvars.iv35, 1
  %exitcond37 = icmp eq i64 %indvars.iv.next36, 3
  br i1 %exitcond37, label %for.cond.cleanup3, label %for.cond5.preheader

for.body8:                                        ; preds = %for.inc, %for.cond5.preheader
  %indvars.iv = phi i64 [ 1, %for.cond5.preheader ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr %q, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4
  %cmp12 = icmp ult i32 %1, %0
  br i1 %cmp12, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body8
  store i32 undef, ptr null, align 16
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.cond.cleanup7, label %for.body8
}

