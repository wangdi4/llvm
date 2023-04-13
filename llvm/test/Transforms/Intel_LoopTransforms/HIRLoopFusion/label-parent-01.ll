; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region -debug-only=hir-loop-fusion < %s 2>&1 | FileCheck %s

; This test case checks that loops <72> and <73> aren't fused because the
; parent loop has a label in between them.

; HIR before fusion

; <0>          BEGIN REGION { }
; <70>               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <71>               |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <14>               |   |   if (i2 == i1)
; <14>               |   |   {
; <15>               |   |      goto inLoop.loopexit77;
; <14>               |   |   }
; <21>               |   |   (%b)[i2] = i2 + 1;
; <71>               |   + END LOOP
; <71>               |   
; <72>               |   
; <72>               |   + DO i2 = 0, 9, 1   <DO_LOOP>
; <34>               |   |   (%a)[i2] = (%b)[i2];
; <72>               |   + END LOOP
; <72>               |   
; <43>               |   inLoop.loopexit77:
; <73>               |   
; <73>               |   + DO i2 = 0, 9, 1   <DO_LOOP>
; <52>               |   |   (%c)[i2] = (%a)[i2];
; <73>               |   + END LOOP
; <70>               + END LOOP
; <70>               
; <69>               ret ;
; <0>          END REGION

; HIR after transformation

; CHECK: Skipping current parent node since it contains a GOTO, a Label, or is a multi-exit loop

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   |   if (i2 == i1)
; CHECK:       |   |   {
; CHECK:       |   |      goto inLoop.loopexit77;
; CHECK:       |   |   }
; CHECK:       |   |   (%b)[i2] = i2 + 1;
; CHECK:       |   + END LOOP
; CHECK:       |   
; CHECK:       |   
; CHECK:       |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:       |   |   (%a)[i2] = (%b)[i2];
; CHECK:       |   + END LOOP
; CHECK:       |   
; CHECK:       |   inLoop.loopexit77:
; CHECK:       |   
; CHECK:       |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:       |   |   (%c)[i2] = (%a)[i2];
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP      
; CHECK:       ret ;
; CHECK: END REGION

; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooPiS_S_i(ptr nocapture noundef %a, ptr nocapture noundef %b, ptr nocapture noundef writeonly %c, i32 noundef %n) {
entry:
  %cmp61 = icmp sgt i32 %n, 0
  br i1 %cmp61, label %for.cond1.preheader.lr.ph, label %cleanup36

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %wide.trip.count74 = zext i32 %n to i64
  br label %for.body4.preheader

for.body4.preheader:                              ; preds = %for.cond.cleanup23, %for.cond1.preheader.lr.ph
  %indvars.iv72 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next73, %for.cond.cleanup23 ]
  br label %for.body4

for.body4:                                        ; preds = %for.body4.preheader, %if.end
  %indvars.iv = phi i64 [ 0, %for.body4.preheader ], [ %indvars.iv.next, %if.end ]
  %cmp5 = icmp eq i64 %indvars.iv, %indvars.iv72
  br i1 %cmp5, label %inLoop.loopexit77, label %if.end

if.end:                                           ; preds = %for.body4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv.next to i32
  store i32 %0, ptr %arrayidx, align 4
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count74
  br i1 %exitcond.not, label %for.body10.preheader, label %for.body4

for.body10.preheader:                             ; preds = %if.end
  br label %for.body10

for.body10:                                       ; preds = %for.body10.preheader, %for.body10
  %indvars.iv64 = phi i64 [ %indvars.iv.next65, %for.body10 ], [ 0, %for.body10.preheader ]
  %arrayidx12 = getelementptr inbounds i32, ptr %b, i64 %indvars.iv64
  %1 = load i32, ptr %arrayidx12, align 4
  %arrayidx15 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv64
  store i32 %1, ptr %arrayidx15, align 4
  %indvars.iv.next65 = add nuw nsw i64 %indvars.iv64, 1
  %exitcond67.not = icmp eq i64 %indvars.iv.next65, 10
  br i1 %exitcond67.not, label %inLoop.loopexit, label %for.body10

inLoop.loopexit:                                  ; preds = %for.body10
  br label %inLoop

inLoop.loopexit77:                                ; preds = %for.body4
  br label %inLoop

inLoop:                                           ; preds = %inLoop.loopexit77, %inLoop.loopexit
  br label %for.body24

for.cond.cleanup23:                               ; preds = %for.body24
  %indvars.iv.next73 = add nuw nsw i64 %indvars.iv72, 1
  %exitcond75.not = icmp eq i64 %indvars.iv.next73, %wide.trip.count74
  br i1 %exitcond75.not, label %cleanup36.loopexit, label %for.body4.preheader

for.body24:                                       ; preds = %inLoop, %for.body24
  %indvars.iv68 = phi i64 [ 0, %inLoop ], [ %indvars.iv.next69, %for.body24 ]
  %arrayidx26 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv68
  %2 = load i32, ptr %arrayidx26, align 4
  %arrayidx29 = getelementptr inbounds i32, ptr %c, i64 %indvars.iv68
  store i32 %2, ptr %arrayidx29, align 4
  %indvars.iv.next69 = add nuw nsw i64 %indvars.iv68, 1
  %exitcond71.not = icmp eq i64 %indvars.iv.next69, 10
  br i1 %exitcond71.not, label %for.cond.cleanup23, label %for.body24

cleanup36.loopexit:                               ; preds = %for.cond.cleanup23
  br label %cleanup36

cleanup36:                                        ; preds = %cleanup36.loopexit, %entry
  ret void
}
