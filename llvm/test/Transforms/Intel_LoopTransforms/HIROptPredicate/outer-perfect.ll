; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s

; Check "if (%conv1 < %k)" being hoisted out of DO i1.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   if (%conv1 < %k)
;       |   {
;       |      + DO i2 = 0, 99, 1   <DO_LOOP>
;       |      |   %0 = (%q)[i2];
;       |      |   (%q)[i2] = i2 + %0;
;       |      + END LOOP
;       |   }
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%conv1 < %k)
; CHECK:       {
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   |   %0 = (%q)[i2];
; CHECK:          |   |   (%q)[i2] = i2 + %0;
; CHECK:          |   + END LOOP
; CHECK:          + END LOOP
; CHECK:       }
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(i32* nocapture %q, i32 %n, i32 %k) local_unnamed_addr #0 {
entry:
  %mul = shl i32 %n, 24
  %sext = mul i32 %mul, %k
  %conv1 = ashr exact i32 %sext, 24
  %cmp2 = icmp slt i32 %conv1, %k
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc9
  ret void

for.body:                                         ; preds = %for.inc9, %entry
  %i.020 = phi i32 [ 0, %entry ], [ %inc10, %for.inc9 ]
  br i1 %cmp2, label %for.body8.preheader, label %for.inc9

for.body8.preheader:                              ; preds = %for.body
  br label %for.body8

for.body8:                                        ; preds = %for.body8.preheader, %for.body8
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body8 ], [ 0, %for.body8.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %1 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %0, %1
  store i32 %add, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.inc9.loopexit, label %for.body8

for.inc9.loopexit:                                ; preds = %for.body8
  br label %for.inc9

for.inc9:                                         ; preds = %for.inc9.loopexit, %for.body
  %inc10 = add nuw nsw i32 %i.020, 1
  %exitcond21 = icmp eq i32 %inc10, 100
  br i1 %exitcond21, label %for.cond.cleanup, label %for.body
}

