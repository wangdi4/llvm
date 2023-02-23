; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region -debug-only=hir-loop-fusion < %s 2>&1 | FileCheck %s

; This test case checks that the two innermost loops are fused because
; the GOTO in the condition won't affect the domination/post-domination
; analysis between them. The HIR looks as follows:

; Function: _Z3fooiPiS_S_S_
;
; BEGIN REGION { }
;       + DO i1 = 0, 999, 1   <DO_MULTI_EXIT_LOOP>
;       |   + DO i2 = 0, 99, 1   <DO_LOOP>
;       |   |   (%a)[i2] = (%b)[i2];
;       |   + END LOOP
;       |   
;       |   
;       |   + DO i2 = 0, 99, 1   <DO_LOOP>
;       |   |   (%c)[i2] = (%a)[i2];
;       |   + END LOOP
;       |   
;       |   if ((%a)[i1] == %m)
;       |   {
;       |      goto cleanup;
;       |   }
;       + END LOOP
;       
;       cleanup:
;       ret ;
; END REGION

; Check that the two innermost loop are fused

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, 999, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:       |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   |   (%a)[i2] = (%b)[i2];
; CHECK:       |   |   (%c)[i2] = (%a)[i2];
; CHECK:       |   + END LOOP
; CHECK:       |
; CHECK:       |   if ((%a)[i1] == %m)
; CHECK:       |   {
; CHECK:       |      goto cleanup;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK:       cleanup:
; CHECK:       ret ;
; CHECK: END REGION


source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @_Z3fooiPiS_S_S_(i32 noundef %m, ptr nocapture noundef noalias %a, ptr nocapture noundef readonly noalias %b, ptr nocapture noundef writeonly noalias %c) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup14
  %indvars.iv52 = phi i64 [ 0, %entry ], [ %indvars.iv.next53, %for.cond.cleanup14 ]
  br label %for.body4.preheader

for.body4.preheader:
  br label %for.body4

for.body15.preheader:                             ; preds = %for.cond.cleanup3
  br label %for.body15

for.body4:                                        ; preds = %for.cond1.preheader, %for.body4
  %indvars.iv = phi i64 [ 0, %for.body4.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 %0, ptr %arrayidx7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.body15.preheader, label %for.body4

for.cond.cleanup14:                               ; preds = %for.body15
  %indvars.iv.next53 = add nuw nsw i64 %indvars.iv52, 1
  %exitcond54.not = icmp eq i64 %indvars.iv.next53, 1000
  br i1 %exitcond54.not, label %cleanup, label %for.cond1.preheader

for.body15:                                       ; preds = %for.body15.preheader, %for.body15
  %indvars.iv48 = phi i64 [ %indvars.iv.next49, %for.body15 ], [ 0, %for.body15.preheader ]
  %arrayidx18 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv48
  %1 = load i32, ptr %arrayidx18
  %arrayidx21 = getelementptr inbounds i32, ptr %c, i64 %indvars.iv48
  store i32 %1, ptr %arrayidx21
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond51.not = icmp eq i64 %indvars.iv.next49, 100
  br i1 %exitcond51.not, label %for.cond.cleanup3, label %for.body15

for.cond.cleanup3:                                ; preds = %for.body4
  %arrayidx9 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv52
  %2 = load i32, ptr %arrayidx9
  %cmp10 = icmp eq i32 %2, %m
  br i1 %cmp10, label %cleanup, label %for.cond.cleanup14

cleanup:                                          ; preds = %for.cond.cleanup3, %for.cond.cleanup14
  ret void
}
