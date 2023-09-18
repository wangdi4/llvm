; RUN: opt -passes="hir-ssa-deconstruction,hir-memory-reduction-sinking,print<hir>"  2>&1 < %s | FileCheck %s

; This test case checks that the reduction inside the If condition is not
; sinked since there is a load after the condition.

; HIR before transformation:

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %0 = (%b)[i1];
;       |   if (%0 > 10)
;       |   {
;       |      %1 = (%a)[5];
;       |      (%a)[5] = %1 + 2;
;       |   }
;       |   %2 = (%a)[5];
;       |   (%b)[i1] = %2 + 2;
;       + END LOOP
; END REGION


; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   %0 = (%b)[i1];
; CHECK:       |   if (%0 > 10)
; CHECK:       |   {
; CHECK:       |      %1 = (%a)[5];
; CHECK:       |      (%a)[5] = %1 + 2;
; CHECK:       |   }
; CHECK:       |   %2 = (%a)[5];
; CHECK:       |   (%b)[i1] = %2 + 2;
; CHECK:       + END LOOP
; CHECK: END REGION

;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooPiS_(ptr nocapture noalias noundef %a, ptr nocapture noundef readonly %b) {
entry:
  %arrayidx2 = getelementptr inbounds i32, ptr %a, i64 5
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.end
  ret void

for.body:                                         ; preds = %entry, %if.end
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ]
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx
  %cmp1 = icmp sgt i32 %0, 10
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %1 = load i32, ptr %arrayidx2
  %add = add nsw i32 %1, 2
  store i32 %add, ptr %arrayidx2
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %2 = load i32, ptr %arrayidx2
  %add2 = add nsw i32 %2, 2
  store i32 %add2, ptr %arrayidx
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}