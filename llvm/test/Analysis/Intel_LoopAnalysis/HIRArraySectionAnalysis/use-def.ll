; RUN: opt < %s -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-array-section-analysis | FileCheck %s
; RUN: opt < %s -aa-pipeline=basic-aa -passes="hir-ssa-deconstruction,print<hir-array-section-analysis>" -disable-output 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %1 = (%p)[i1 + 200];
;       |   (%p)[i1] = i1 + %1;
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { }
; CHECK:    + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:    %p: (USEDEF) [i1,i1 + 200:0:299]
; CHECK:    + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %p) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = add nuw nsw i64 %indvars.iv, 200
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %0
  %1 = load i32, i32* %arrayidx, align 4
  %2 = trunc i64 %indvars.iv to i32
  %add1 = add nsw i32 %1, %2
  %arrayidx3 = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  store i32 %add1, i32* %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

