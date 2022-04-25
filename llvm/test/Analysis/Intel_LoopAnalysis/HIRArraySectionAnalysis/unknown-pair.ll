; RUN: opt < %s -hir-ssa-deconstruction -scoped-noalias-aa -hir-runtime-dd -analyze -enable-new-pm=0 -hir-array-section-analysis | FileCheck %s
; RUN: opt < %s -aa-pipeline="basic-aa,scoped-noalias-aa" -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir-array-section-analysis>" -disable-output 2>&1 | FileCheck %s

; Check that result for %p and %q are * bacause they may alias. Check that result is available after RTDD.

; BEGIN REGION { }
;       + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;       |   %0 = (%q)[i1];
;       |   (%p)[i1] = i1 + %0;
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { }
; CHECK:    + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MVTag: 14>
; CHECK:    %q: (USE) [i1:0:%n + -1]
; CHECK:    %p: (DEF) [i1:0:%n + -1]
; CHECK:    + END LOOP
; CHECK:    + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:    %q: *
; CHECK:    %p: *
; CHECK:    + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %p, i32* nocapture readonly %q, i64 %n) {
entry:
  %cmp9 = icmp eq i64 %n, 0
  br i1 %cmp9, label %for.cond.cleanup, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.010 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %q, i64 %i.010
  %0 = load i32, i32* %arrayidx, align 4
  %1 = trunc i64 %i.010 to i32
  %conv1 = add i32 %0, %1
  %arrayidx2 = getelementptr inbounds i32, i32* %p, i64 %i.010
  store i32 %conv1, i32* %arrayidx2, align 4
  %inc = add nuw i64 %i.010, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

