
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test checks that the partial unswitching for inveriant predicates won't
; produce a compiler failure when the candidate predicate is at a position
; larger than 2.
;
; NOTE: This test case can be improved for cleaning the redundant nested
; conditions.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   if (i1 == %n & %t > 1)
;       |   {
;       |      + DO i2 = 0, 99, 1   <DO_LOOP>
;       |      |   if (i1 == %n & i2 == %n & %t > 1)
;       |      |   {
;       |      |      (%a)[i2] = i1;
;       |      |   }
;       |      + END LOOP
;       |   }
;       + END LOOP
; END REGION

; HIR after transformation

; TODO: The duplicated nested 'if (%t > 1)' needs be cleanup during
; HLNodeUtils::removeRedundantNodes and/or when analyzing if two conditions
; can be combined.

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%t > 1)
; CHECK:       {
; CHECK:          if (%t > 1)
; CHECK:          {
; CHECK:             + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:             |   if (i1 == %n)
; CHECK:             |   {
; CHECK:             |      + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:             |      |   if (i1 == %n & i2 == %n)
; CHECK:             |      |   {
; CHECK:             |      |      (%a)[i2] = i1;
; CHECK:             |      |   }
; CHECK:             |      + END LOOP
; CHECK:             |   }
; CHECK:             + END LOOP
; CHECK:          }
; CHECK:       }
; CHECK: END REGION


;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooPiii(ptr nocapture noundef writeonly %a, i32 noundef %t, i32 noundef %n) {
entry:
  %cmp2 = icmp sgt i32 %t, 1
  %0 = zext i32 %n to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc12
  ret void

for.body:                                         ; preds = %entry, %for.inc12
  %i.025 = phi i32 [ 0, %entry ], [ %inc13, %for.inc12 ]
  %cmp1 = icmp eq i32 %i.025, %n
  %or.cond = and i1 %cmp1, %cmp2
  br i1 %or.cond, label %for.body6.preheader, label %for.inc12

for.body6.preheader:                              ; preds = %for.body
  br label %for.body6

for.body6:                                        ; preds = %for.body6.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body6.preheader ]
  %cmp7.not = icmp eq i64 %indvars.iv, %0
  %and1 = and i1 %cmp1, %cmp7.not
  %and = and i1 %and1, %cmp2
  br i1 %and, label %if.then10, label %for.inc

if.then10:                                        ; preds = %for.body6
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 %i.025, ptr %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body6, %if.then10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.inc12.loopexit, label %for.body6

for.inc12.loopexit:                               ; preds = %for.inc
  br label %for.inc12

for.inc12:                                        ; preds = %for.inc12.loopexit, %for.body
  %inc13 = add nuw nsw i32 %i.025, 1
  %exitcond26.not = icmp eq i32 %inc13, 100
  br i1 %exitcond26.not, label %for.cond.cleanup, label %for.body
}

