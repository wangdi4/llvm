; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region -debug-only=hir-loop-fusion < %s 2>&1 | FileCheck %s

; This test case checks that loops <78> and <79> aren't fused because loop
; <76> is multi-exit.

; NOTE: There still potential to fuse these two loops. This is future work.

; HIR before transformation

; <0>          BEGIN REGION { }
; <76>               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <77>               |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <18>               |   |   if ((%a)[i1] == (%c)[i2])
; <18>               |   |   {
; <19>               |   |      goto for.inc38.loopexit83;
; <18>               |   |   }
; <25>               |   |   (%b)[i2] = i2 + 1;
; <77>               |   + END LOOP
; <77>               |   
; <78>               |   
; <78>               |   + DO i2 = 0, 9, 1   <DO_LOOP>
; <39>               |   |   (%a)[i1 + i2] = (%b)[i2];
; <78>               |   + END LOOP
; <78>               |   
; <79>               |   
; <79>               |   + DO i2 = 0, 9, 1   <DO_LOOP>
; <54>               |   |   (%c)[i1 + i2] = (%a)[i2];
; <79>               |   + END LOOP
; <79>               |   
; <63>               |   for.inc38.loopexit83:
; <76>               + END LOOP
; <76>               
; <75>               ret ;
; <0>          END REGION

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   |   if ((%a)[i1] == (%c)[i2])
; CHECK:       |   |   {
; CHECK:       |   |      goto for.inc38.loopexit83;
; CHECK:       |   |   }
; CHECK:       |   |   (%b)[i2] = i2 + 1;
; CHECK:       |   + END LOOP
; CHECK:       |   
; CHECK:       |   
; CHECK:       |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:       |   |   (%a)[i1 + i2] = (%b)[i2];
; CHECK:       |   + END LOOP
; CHECK:       |   
; CHECK:       |   
; CHECK:       |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:       |   |   (%c)[i1 + i2] = (%a)[i2];
; CHECK:       |   + END LOOP
; CHECK:       |   
; CHECK:       |   for.inc38.loopexit83:
; CHECK:       + END LOOP
; CHECK:       ret ;
; CHECK: END REGION


source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooPiS_S_i(ptr nocapture noundef noalias %a, ptr nocapture noundef noalias %b, ptr nocapture noundef noalias %c, i32 noundef %n) {
entry:
  %cmp67 = icmp sgt i32 %n, 0
  br i1 %cmp67, label %for.cond1.preheader.lr.ph, label %cleanup40

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %wide.trip.count80 = zext i32 %n to i64
  br label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.inc38, %for.cond1.preheader.lr.ph
  %indvars.iv78 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next79, %for.inc38 ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv78
  br label %for.body4

for.body4:                                        ; preds = %for.body4.lr.ph, %if.end
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %if.end ]
  %0 = load i32, ptr %arrayidx, align 4
  %arrayidx6 = getelementptr inbounds i32, ptr %c, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx6, align 4
  %cmp7 = icmp eq i32 %0, %1
  br i1 %cmp7, label %for.inc38.loopexit83, label %if.end

if.end:                                           ; preds = %for.body4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx9 = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %2 = trunc i64 %indvars.iv.next to i32
  store i32 %2, ptr %arrayidx9, align 4
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count80
  br i1 %exitcond.not, label %for.body14.preheader, label %for.body4

for.body14.preheader:                             ; preds = %if.end
  br label %for.body14

for.body14:                                       ; preds = %for.body14.preheader, %for.body14
  %indvars.iv70 = phi i64 [ %indvars.iv.next71, %for.body14 ], [ 0, %for.body14.preheader ]
  %arrayidx16 = getelementptr inbounds i32, ptr %b, i64 %indvars.iv70
  %3 = load i32, ptr %arrayidx16, align 4
  %4 = add nuw nsw i64 %indvars.iv70, %indvars.iv78
  %arrayidx19 = getelementptr inbounds i32, ptr %a, i64 %4
  store i32 %3, ptr %arrayidx19, align 4
  %indvars.iv.next71 = add nuw nsw i64 %indvars.iv70, 1
  %exitcond73.not = icmp eq i64 %indvars.iv.next71, 10
  br i1 %exitcond73.not, label %for.body28.preheader, label %for.body14

for.body28.preheader:                             ; preds = %for.body14
  br label %for.body28

for.body28:                                       ; preds = %for.body28.preheader, %for.body28
  %indvars.iv74 = phi i64 [ %indvars.iv.next75, %for.body28 ], [ 0, %for.body28.preheader ]
  %arrayidx30 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv74
  %5 = load i32, ptr %arrayidx30, align 4
  %6 = add nuw nsw i64 %indvars.iv74, %indvars.iv78
  %arrayidx33 = getelementptr inbounds i32, ptr %c, i64 %6
  store i32 %5, ptr %arrayidx33, align 4
  %indvars.iv.next75 = add nuw nsw i64 %indvars.iv74, 1
  %exitcond77.not = icmp eq i64 %indvars.iv.next75, 10
  br i1 %exitcond77.not, label %for.inc38.loopexit, label %for.body28

for.inc38.loopexit:                               ; preds = %for.body28
  br label %for.inc38

for.inc38.loopexit83:                             ; preds = %for.body4
  br label %for.inc38

for.inc38:                                        ; preds = %for.inc38.loopexit83, %for.inc38.loopexit
  %indvars.iv.next79 = add nuw nsw i64 %indvars.iv78, 1
  %exitcond81.not = icmp eq i64 %indvars.iv.next79, %wide.trip.count80
  br i1 %exitcond81.not, label %cleanup40.loopexit, label %for.body4.lr.ph

cleanup40.loopexit:                               ; preds = %for.inc38
  br label %cleanup40

cleanup40:                                        ; preds = %cleanup40.loopexit, %entry
  ret void
}