; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -S -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -S -disable-output < %s 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;       |   |   if ((%n /u %x) >u 10)
;       |   |   {
;       |   |      (%p)[i1 + i2] = i1 + i2;
;       |   |   }
;       |   |   else
;       |   |   {
;       |   |      (%p)[i1] = 1;
;       |   |   }
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   if (%n != 0)
; CHECK:       |   {
; CHECK:       |      if ((%n /u %x) >u 10)
; CHECK:       |      {
; CHECK:       |         + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:       |         |   (%p)[i1 + i2] = i1 + i2;
; CHECK:       |         + END LOOP
; CHECK:       |      }
; CHECK:       |      else
; CHECK:       |      {
; CHECK:       |         + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:       |         |   (%p)[i1] = 1;
; CHECK:       |         + END LOOP
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %x, i32 %n, i32* nocapture %p) local_unnamed_addr #0 {
entry:
  %cmp223 = icmp eq i32 %n, 0
  %wide.trip.count = zext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvars.iv28 = phi i64 [ 0, %entry ], [ %indvars.iv.next29, %for.cond.cleanup3 ]
  br i1 %cmp223, label %for.cond.cleanup3, label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader
  %div = udiv i32 %n, %x
  %cmp5 = icmp ugt i32 %div, 10
  %arrayidx8 = getelementptr inbounds i32, i32* %p, i64 %indvars.iv28
  %0 = trunc i64 %indvars.iv28 to i32
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3.loopexit:                       ; preds = %for.inc
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.cond1.preheader
  %indvars.iv.next29 = add nuw nsw i64 %indvars.iv28, 1
  %exitcond30 = icmp eq i64 %indvars.iv.next29, 100
  br i1 %exitcond30, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.inc, %for.body4.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp5, label %if.then, label %if.else

if.then:                                          ; preds = %for.body4
  %1 = trunc i64 %indvars.iv to i32
  %add = add i32 %1, %0
  %idxprom = zext i32 %add to i64
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %idxprom
  store i32 %add, i32* %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body4
  store i32 1, i32* %arrayidx8, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup3.loopexit, label %for.body4
}

